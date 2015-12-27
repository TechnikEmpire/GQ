/*
* Adapted from https://github.com/google/gumbo-parser/blob/master/examples/serialize.cc
*
* Copyright (c) 2015 Kevin B. Hendricks, Stratford, Ontario,  All Rights Reserved.
* Copyright (c) 2015 Jesse Nicholson
* loosely based on a greatly simplified version of BeautifulSoup4 decode() routine
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "Serializer.hpp"
#include "Node.hpp"
#include "Util.hpp"

namespace gq
{

	const std::unordered_set<boost::string_ref, StringRefHash> Serializer::EmptyTags =
	{
		{ u8"area" },
		{ u8"base" },
		{ u8"basefont" },
		{ u8"bgsound" },
		{ u8"br" },
		{ u8"command" },
		{ u8"col" },
		{ u8"embed" },
		{ u8"event-source" },
		{ u8"frame" },
		{ u8"hr" },
		{ u8"image" },
		{ u8"img" },
		{ u8"input" },
		{ u8"keygen" },
		{ u8"link" },
		{ u8"menuitem" },
		{ u8"meta" },
		{ u8"param" },
		{ u8"source" },
		{ u8"spacer" },
		{ u8"track" },
		{ u8"wbr" }
	};

	const std::unordered_set<boost::string_ref, StringRefHash> Serializer::SpecialHandling =
	{
		{ u8"html" },
		{ u8"body" }
	};

	Serializer::Serializer()
	{
	}

	Serializer::~Serializer()
	{
	}

	std::string Serializer::Serialize(const Node* node, const NodeMutationCollection* mutationCollection)
	{
		return Serialize(node->m_node, mutationCollection);
	}

	std::string Serializer::Serialize(const GumboNode* node, const NodeMutationCollection* mutationCollection)
	{		
		// special case the document node
		if (node->type == GUMBO_NODE_DOCUMENT)
		{
			std::string docResults = BuildDocType(node);
			docResults.append(SerializeContent(node, false, mutationCollection));

			return docResults;
		}

		std::string close;
		std::string closeTag;
		std::string atts;
		std::string tagname;

		std::string tagOpeningText;

		// Check to see if the supplied collection is valid and if the current node can be found in
		// it.
		bool foundNodeInUserCollection = false;

		if (mutationCollection && mutationCollection->Contains(node))
		{
			foundNodeInUserCollection = true;
		}

		if (foundNodeInUserCollection && mutationCollection->m_onTagStart)
		{
			auto result = mutationCollection->m_onTagStart(node->v.element.tag);

			// If the user opted to not start building the tag, then the user doesn't want it.
			// Return the empty string.
			if (result == false)
			{
				return tagOpeningText;
			}
		}

		tagname = GetTagName(node);
		tagOpeningText.append(u8"<" + tagname);

		boost::string_ref tagNameStrRef(tagname);

		bool needSpecialHandling = (SpecialHandling.find(tagNameStrRef) != SpecialHandling.end());
		bool isEmptyTag = (EmptyTags.find(tagNameStrRef) != EmptyTags.end());

		// build attributes into a single string  
		const GumboVector* attribs = &node->v.element.attributes;		
		
		for (size_t i = 0; i < attribs->length; ++i)
		{
			GumboAttribute* attribute = static_cast<GumboAttribute*>(attribs->data[i]);
			
			if (foundNodeInUserCollection && mutationCollection->m_onTagAttribute)
			{
				boost::string_ref attribName;
				boost::string_ref attribValue;

				if (attribute->original_name.length > 0)
				{
					attribName = boost::string_ref(attribute->original_name.data, attribute->original_name.length);
				}

				if (attribute->original_value.length > 0)
				{
					attribValue = Util::TrimEnclosingQuotes(boost::string_ref(attribute->original_value.data, attribute->original_value.length));
				}				

				if (attribName.size() > 0)
				{
					mutationCollection->m_onTagAttribute(node->v.element.tag, atts, attribName, attribValue);
				}

				continue;
			}
			
			atts.append(BuildAttributes(attribute));
		}

		// determine closing tag type
		if (isEmptyTag) {
			close = u8"/";
		}
		else
		{
			closeTag = u8"</" + tagname + u8">";
		}

		// serialize your contents
		std::string contents;
		
		if (foundNodeInUserCollection && mutationCollection->m_onTagContent)
		{
			auto result = mutationCollection->m_onTagContent(node->v.element.tag, contents);

			if (contents.size() > 0)
			{
				if (result == true)
				{
					// The supplied user content should replace text nodes in the content, if any.
					// Everything else should be appended normally.
					contents.append(SerializeContent(node, true, mutationCollection));
				}
				else if (result == false)
				{
					// Only the user data is to be appended as the contents of this node. That means
					// we don't need to call SerializeContent at all.
				}
			}
			else 
			{
				// The user returned nothing, so as promised, serialize normally.
				contents = SerializeContent(node, false, mutationCollection);
			}					
		}
		else
		{
			contents = SerializeContent(node, false, mutationCollection);
		}		

		if (needSpecialHandling) {
			boost::trim(contents);
			contents.append(u8"\n");
		}

		// build results
		std::string results;

		results.append(tagOpeningText + atts + close + u8">");

		if (needSpecialHandling)
		{
			results.append(u8"\n");
		}

		results.append(contents);
		results.append(closeTag);

		if (needSpecialHandling)
		{
			results.append(u8"\n");
		}

		return results;
	}

	std::string Serializer::SerializeContent(const Node* node, const bool omitText, const NodeMutationCollection* mutationCollection)
	{
		return SerializeContent(node->m_node, omitText, mutationCollection);
	}

	std::string Serializer::SerializeContent(const GumboNode* node, const bool omitText, const NodeMutationCollection* mutationCollection)
	{
		std::string tagNameStr = GetTagName(node);
		boost::string_ref tagNameStrRef(tagNameStr);
		std::string contents;

		// build up result for each child, recursively if need be
		const GumboVector* children = &node->v.element.children;

		for (size_t i = 0; i < children->length; ++i)
		{
			GumboNode* child = static_cast<GumboNode*> (children->data[i]);

			switch (child->type)
			{
				case GUMBO_NODE_TEXT:
				{
					if (!omitText) 
					{
						contents.append(std::string(child->v.text.original_text.data, child->v.text.original_text.length));
					}					
				}
				break;

				case GUMBO_NODE_ELEMENT:
				case GUMBO_NODE_TEMPLATE:
				{
					contents.append(Serialize(child, mutationCollection));
				}
				break;

				case GUMBO_NODE_WHITESPACE:
				{
					// keep all whitespace to keep as close to original as possible
					contents.append(std::string(child->v.text.text));
				}
				break;

				case GUMBO_NODE_COMMENT:
				case GUMBO_NODE_CDATA:
				{
					contents.append(std::string(child->v.text.original_text.data, child->v.text.original_text.length));
				}
				break;
			}
		}

		return contents;
	}

	std::string Serializer::GetTagName(const GumboNode* node)
	{
		boost::string_ref tagName;

		switch (node->type)
		{
		case GUMBO_NODE_DOCUMENT:
		{
			tagName = u8"document";
		}
		break;

		default:
		{
			tagName = boost::string_ref(gumbo_normalized_tagname(node->v.element.tag));
		}
		break;
		}

		// Handle unknown tag right here.
		if (tagName.empty())
		{
			// If string length is zero, then the string is null
			if (node->v.element.original_tag.length == 0)
			{
				boost::string_ref unknownTag(node->v.element.original_tag.data, node->v.element.original_tag.length);
				auto start = unknownTag.find_first_not_of(u8"><\\ \t\r\n");

				if (start == boost::string_ref::npos)
				{
					return std::string();
				}

				unknownTag = unknownTag.substr(start);

				auto end = unknownTag.find_first_of(u8"><\\ \t\r\n");

				if (end != boost::string_ref::npos)
				{
					unknownTag = unknownTag.substr(0, end);
				}
			}
		}

		return tagName.to_string();
	}

	std::string Serializer::BuildDocType(const GumboNode* node)
	{
		std::string results;

		if (node->v.document.has_doctype)
		{
			results.append(u8"<!DOCTYPE ");
			results.append(node->v.document.name);

			if (node->v.document.public_identifier != nullptr)
			{
				boost::string_ref pi(node->v.document.public_identifier);

				if (!pi.empty())
				{
					results.append(u8" PUBLIC \"");
					results.append(node->v.document.public_identifier);
					results.append(u8"\" \"");
					results.append(node->v.document.system_identifier);
					results.append(u8"\"");
				}
			}

			results.append(u8">\n");
		}

		return results;
	}

	std::string Serializer::BuildAttributes(const GumboAttribute* at)
	{
		std::string atts(u8" ");
		atts.append(at->original_name.data, at->original_name.length);

		boost::string_ref attValue(at->original_value.data, at->original_value.length);		

		if (!attValue.empty())
		{
			atts.append(u8"=");

			atts.append(attValue.to_string());
		}

		return atts;
	}

} /* namespace gq */
