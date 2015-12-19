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

#include "GQSerializer.hpp"
#include "GQNode.hpp"

namespace gq
{

	const std::unordered_set<boost::string_ref, GQSerializer::StringRefHasher> GQSerializer::EmptyTags =
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

	const std::unordered_set<boost::string_ref, GQSerializer::StringRefHasher> GQSerializer::SpecialHandling =
	{
		{ u8"html" },
		{ u8"body" }
	};

	GQSerializer::GQSerializer()
	{
	}

	GQSerializer::~GQSerializer()
	{
	}

	std::string GQSerializer::Serialize(const GQNode* node)
	{
		return Serialize(node->m_node);
	}

	std::string GQSerializer::Serialize(const GumboNode* node)
	{
		// special case the document node
		if (node->type == GUMBO_NODE_DOCUMENT)
		{
			std::string docResults = BuildDocType(node);
			docResults.append(SerializeContent(node));

			return docResults;
		}

		std::string close;
		std::string closeTag;
		std::string atts;
		std::string tagname = GetTagName(node);

		boost::string_ref tagNameStrRef(tagname);

		bool needSpecialHandling = (SpecialHandling.find(tagNameStrRef) != SpecialHandling.end());
		bool isEmptyTag = (EmptyTags.find(tagNameStrRef) != EmptyTags.end());

		// build attr string  
		const GumboVector* attribs = &node->v.element.attributes;
		for (size_t i = 0; i < attribs->length; ++i)
		{
			GumboAttribute* at = static_cast<GumboAttribute*>(attribs->data[i]);
			atts.append(BuildAttributes(at));
		}

		// determine closing tag type
		if (isEmptyTag) {
			close = "/";
		}
		else
		{
			closeTag = "</" + tagname + ">";
		}

		// serialize your contents
		std::string contents = SerializeContent(node);

		if (needSpecialHandling) {
			boost::trim(contents);
			contents.append("\n");
		}

		// build results
		std::string results;

		results.append("<" + tagname + atts + close + ">");

		if (needSpecialHandling)
		{
			results.append("\n");
		}

		results.append(contents);
		results.append(closeTag);

		if (needSpecialHandling)
		{
			results.append("\n");
		}

		return results;
	}

	std::string GQSerializer::SerializeContent(const GQNode* node)
	{
		return SerializeContent(node->m_node);
	}

	std::string GQSerializer::SerializeContent(const GumboNode* node)
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
				contents.append(std::string(child->v.text.original_text.data, child->v.text.original_text.length));
			}
			break;

			case GUMBO_NODE_ELEMENT:
			case GUMBO_NODE_TEMPLATE:
			{
				contents.append(Serialize(child));
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

	std::string GQSerializer::GetTagName(const GumboNode* node)
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

		// Handle unknown tag right here rather than delegating needlessly
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

	std::string GQSerializer::BuildDocType(const GumboNode* node)
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

	std::string GQSerializer::BuildAttributes(const GumboAttribute* at)
	{
		std::string atts(u8" ");
		atts.append(at->original_name.data, at->original_name.length);

		boost::string_ref attValue(at->original_value.data, at->original_value.length);

		if (!attValue.empty())
		{
			atts.append(u8"=");

			atts.append(at->original_value.data, at->original_value.length);
		}

		return atts;
	}

} /* namespace gq */
