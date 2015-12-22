/*
* This is a heavily modified fork of gumbo-query by Hoping White aka LazyTiger.
* The original software can be found at: https://github.com/lazytiger/gumbo-query
*
* gumbo-query is based on cascadia, written by Andy Balholm.
*
* Copyright (c) 2011 Andy Balholm. All rights reserved.
* Copyright (c) 2015 Hoping White aka LazyTiger (hoping@baimashi.com)
* Copyright (c) 2015 Jesse Nicholson
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include "GQNode.hpp"
#include "GQUtil.hpp"
#include "GQSelection.hpp"
#include "GQParser.hpp"
#include "GQTreeMap.hpp"
#include "GQSpecialTraits.hpp"
#include "GQSerializer.hpp"
#include <typeinfo>

namespace gq
{

	std::unique_ptr<GQNode> GQNode::Create(const GumboNode* node, GQTreeMap* map, const std::string& parentId, const size_t indexWithinParent, GQNode* parent)
	{
		std::string newNodeId = parentId + std::string(u8"A") + std::to_string(indexWithinParent);
		auto newNode = std::unique_ptr<GQNode>{ new GQNode(node, newNodeId, indexWithinParent, parent) };

		#ifndef NDEBUG		
			assert(map != nullptr && u8"In GQNode::Create(const GumboNode*, GQTreeMap*, const std::string&, const size_t, GQNode*) - Cannot initialize a GQNode without a valid GQTreeMap* pointer. GQTreeMap* is nullptr.");
		#else		
			if (map == nullptr) { throw std::runtime_error(u8"In GQNode::Create(const GumboNode*, GQTreeMap*, const std::string&, const size_t, GQNode*) - Cannot initialize a GQNode without a valid GQTreeMap* pointer. GQTreeMap* is nullptr."); }
		#endif	

		newNode->m_rootTreeMap = map;

		newNode->BuildAttributes();
		newNode->BuildChildren();

		return newNode;
	}

	GQNode::GQNode()
	{
		m_parent = nullptr;
	}

	GQNode::GQNode(const GumboNode* node, const std::string newUniqueId, const size_t indexWithinParent, GQNode* parent) :
		m_node(node), 
		m_nodeUniqueId(std::move(newUniqueId)),
		m_indexWithinParent(indexWithinParent), 
		m_parent(parent)
	{		
		#ifndef NDEBUG
		assert(node != nullptr && u8"In GQNode::GQNode(const GumboNode*) - Cannot construct a GQNode around a nullptr.");		
		#else
		if (node == nullptr) { throw std::runtime_error(u8"In GQNode::GQNode(const GumboNode*) - Cannot construct a GQNode around a nullptr."); }		
		#endif			
	}	

	GQNode::~GQNode()
	{

	}

	GQNode* GQNode::GetParent() const
	{
		return m_parent;
	}

	const size_t GQNode::GetIndexWithinParent() const
	{
		return m_indexWithinParent;
	}

	const size_t GQNode::GetNumChildren() const
	{
		return m_children.size();
	}

	const GQNode* GQNode::GetChildAt(const size_t index) const
	{
		if (index >= m_children.size())
		{
			throw std::runtime_error(u8"In GQNode::GetChildAt(const size_t) - Supplied index is out of bounds.");
		}

		return m_children[index].get();
	}

	const bool GQNode::HasAttribute(const std::string& attributeName) const
	{
		boost::string_ref ref(attributeName);
		return HasAttribute(ref);
	}

	const bool GQNode::HasAttribute(const boost::string_ref attributeName) const
	{
		auto search = m_attributes.find(attributeName);
		return search != m_attributes.end();
	}

	const bool GQNode::IsEmpty() const
	{
		if (m_children.size() > 0)
		{
			return false;
		}

		for (size_t i = 0; i < m_node->v.element.children.length; ++i)
		{
			const GumboNode* cnode = static_cast<GumboNode*>(m_node->v.element.children.data[i]);

			if (cnode->type == GUMBO_NODE_TEXT || cnode->type == GUMBO_NODE_TEMPLATE)
			{
				return false;
			}
		}

		return true;
	}

	boost::string_ref GQNode::GetAttributeValue(const boost::string_ref attributeName) const
	{			
		auto res = m_attributes.find(attributeName);
		if (res != m_attributes.end())
		{
			return res->second;
		}

		return boost::string_ref();
	}

	std::string GQNode::GetText() const
	{
		return GQUtil::NodeText(this);
	}

	std::string GQNode::GetOwnText() const
	{
		return GQUtil::NodeOwnText(this);
	}

	const size_t GQNode::GetStartPosition() const
	{
		switch (m_node->type)
		{
			case GUMBO_NODE_ELEMENT:
			{				
				return m_node->v.element.start_pos.offset + m_node->v.element.original_tag.length;
			}
			break;

			case GUMBO_NODE_TEXT:
			{
				return m_node->v.text.start_pos.offset;
			}
			break;

			default:
				// XXX TODO - I don't like the idea of returning zero since it implies a valid position.
				// Perhaps change the return values to be int32_t and return -1 for invalid nodes. However,
				// matching shouldn't work against non element and non text nodes, so there theoretically
				// could never be a situation where this code is reached. For now this is to shut up the
				// compiler.
				return 0;
		}
	}

	const size_t GQNode::GetEndPosition() const
	{
		switch (m_node->type)
		{
			case GUMBO_NODE_ELEMENT:
			{
				return m_node->v.element.end_pos.offset;
			}
			break;

			case GUMBO_NODE_TEXT:
			{
				return m_node->v.text.original_text.length + GetStartPosition();
			}
			break;

			default:
				// XXX TODO - I don't like the idea of returning zero since it implies a valid position.
				// Perhaps change the return values to be int32_t and return -1 for invalid nodes. However,
				// matching shouldn't work against non element and non text nodes, so there theoretically
				// could never be a situation where this code is reached. For now this is to shut up the
				// compiler.
				return 0;
		}
	}

	const size_t GQNode::GetStartOuterPosition() const
	{
		switch (m_node->type)
		{
			case GUMBO_NODE_ELEMENT:
			{
				return m_node->v.element.start_pos.offset;
			}
			break;

			case GUMBO_NODE_TEXT:
			{
				return m_node->v.text.start_pos.offset;
			}
			break;

			default:
				// XXX TODO - I don't like the idea of returning zero since it implies a valid position.
				// Perhaps change the return values to be int32_t and return -1 for invalid nodes. However,
				// matching shouldn't work against non element and non text nodes, so there theoretically
				// could never be a situation where this code is reached. For now this is to shut up the
				// compiler.
				return 0;
		}
	}

	const size_t GQNode::GetEndOuterPosition() const
	{
		switch (m_node->type)
		{
			case GUMBO_NODE_ELEMENT:
			{
				return m_node->v.element.end_pos.offset + m_node->v.element.original_end_tag.length;
			}
			break;

			case GUMBO_NODE_TEXT:
			{
				return m_node->v.text.original_text.length + GetStartPosition();
			}
			break;

			default:
				// XXX TODO - I don't like the idea of returning zero since it implies a valid position.
				// Perhaps change the return values to be int32_t and return -1 for invalid nodes. However,
				// matching shouldn't work against non element and non text nodes, so there theoretically
				// could never be a situation where this code is reached. For now this is to shut up the
				// compiler.
				return 0;
		}
	}

	boost::string_ref GQNode::GetTagName() const
	{	
		if (m_node->type != GUMBO_NODE_ELEMENT)
		{
			return boost::string_ref();
		}

		return boost::string_ref(gumbo_normalized_tagname(m_node->v.element.tag));
	}

	const GumboTag& GQNode::GetTag() const
	{
		return m_node->v.element.tag;
	}

	const GQSelection GQNode::Find(const std::string& selectorString) const
	{
		GQParser parser;

		auto selector = parser.CreateSelector(selectorString);

		return Find(selector);
	}

	const GQSelection GQNode::Find(const SharedGQSelector& selector) const
	{
		// This NO_OP can be ignored. I've kept it for verifying that the indexed
		// method returns the same results as full recursive searching. This full
		// recurise searching is so slow, observing it might actually kill you.
		// It's so bad that my mother told me she was disappointed in the man I've 
		// become when I first wrote it. My father hasn't spoken to me since.
		// Bjarne Stroustrup refunded me on my purchases of his books just so that
		// he didn't have any association with me. I hear he wakes up in cold sweats
		// gripped in fear, questioning his life over this code. When he says that
		// "most people don't use C++ correctly", he's actually just referring to me.
		// Don't use it. Ever.
		#ifdef GQ_FIND_NO_OP
		std::vector<UniqueGQNode> results;
		selector->MatchAll(shared_from_this(), results);

		#ifdef GQ_VERBOSE_DEBUG_NFO
		for (auto& start = results.begin(); start != results.end(); ++start)
		{
			// Print out the matches.
			std::cout << GQSerializer::Serialize(start->get()) << std::endl;
		}
		#endif

		GQSelection selection(results);

		return selection;
		#else

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_DEBUG_NFO
					std::cout << u8"GQNode::Find(const SharedGQSelector&)" << std::endl;
			#endif
		#endif

		std::vector<const GQNode*> matchResults;

		const auto& traits = selector->GetMatchTraits();

		// The collected map ensure that we don't store duplicate matches. Any time a
		// match is made, it's pushed to the collected map.
		FastAttributeMap collected;

		for (auto& traitsIt = traits.begin(); traitsIt != traits.end(); ++traitsIt)
		{

			#ifndef NDEBUG
				#ifdef GQ_VERBOSE_DEBUG_NFO
					std::cout << u8"In GQNode::Find(const SharedGQSelector&) - Finding potential matches at scope " << GetUniqueId() << u8" by trait: " << traitsIt->first << u8" ::: " << traitsIt->second << std::endl;
			#endif
			#endif

			if (traitsIt->first.size() == 0)
			{
				#ifndef NDEBUG
					#ifdef GQ_VERBOSE_DEBUG_NFO
							std::cout << u8"In GQNode::Find(const SharedGQSelector&) - Got trait with empty key. Skipping..." << std::endl;
					#endif
				#endif
				continue;
			}

			const std::vector< const GQNode* >* fromTrait = nullptr;

			if (traitsIt->second.size() == 0)
			{
				fromTrait = m_rootTreeMap->Get(GetUniqueId(), traitsIt->first);
			}
			else
			{
				fromTrait = m_rootTreeMap->Get(GetUniqueId(), traitsIt->first, traitsIt->second);
			}

			#ifndef NDEBUG
				#ifdef GQ_VERBOSE_DEBUG_NFO
					if (fromTrait != nullptr)
					{
						std::cout << u8"In GQNode::Find(const SharedGQSelector&) - Got " << fromTrait->size() << u8" candidates from trait." << std::endl;
					}
					else
					{
						std::cout << u8"In GQNode::Find(const SharedGQSelector&) - Got zero candidates from trait." << std::endl;
					}
				#endif
			#endif

			if (fromTrait != nullptr)
			{
				auto tSize = fromTrait->size();

				if (matchResults.capacity() < tSize)
				{
					matchResults.reserve(matchResults.capacity() + tSize);
				}

				for (size_t i = 0; i < tSize; ++i)
				{
					// It's actually significantly faster to simply match then search for duplicates, rather than eliminate duplicates
					// first and then attempt a match.
					auto* pNode = (*fromTrait)[i];

					auto matchTest = selector->Match(pNode);
					if (matchTest)
					{
						auto* matchedNode = matchTest.GetResult();

						if (collected.find(matchedNode->GetUniqueId()) == collected.end())
						{							
							collected.insert({ matchedNode->GetUniqueId(), matchedNode->GetUniqueId() });
							matchResults.emplace_back(std::move(matchedNode));
						}
					}
				}
			}
		}

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_DEBUG_NFO
				std::cout << u8"Returning " << matchResults.size() << u8" matches for selector " << selector->GetOriginalSelectorString() << u8"." << std::endl;
			#endif
		#endif

		return GQSelection(matchResults);
		#endif
	}

	void GQNode::Each(const std::string& selectorString, std::function<void(const GQNode* node)> func) const
	{
		GQParser parser;

		auto selector = parser.CreateSelector(selectorString);

		Each(selector, func);
	}

	void GQNode::Each(const SharedGQSelector& selector, std::function<void(const GQNode* node)> func) const
	{
		const auto& traits = selector->GetMatchTraits();

		// The collected map ensure that we don't store duplicate matches. Any time a
		// match is made, it's pushed to the collected map.
		FastAttributeMap collected;

		for (auto& traitsIt = traits.begin(); traitsIt != traits.end(); ++traitsIt)
		{

			#ifndef NDEBUG
				#ifdef GQ_VERBOSE_DEBUG_NFO
					std::cout << u8"In GQNode::Each(const SharedGQSelector&, std::function<void(const GQNode* node)>) - Finding potential matches at scope " << GetUniqueId() << u8" by trait: " << traitsIt->first << u8" ::: " << traitsIt->second << std::endl;
				#endif
			#endif

			if (traitsIt->first.size() == 0)
			{
				#ifndef NDEBUG
					#ifdef GQ_VERBOSE_DEBUG_NFO
						std::cout << u8"In GQNode::Each(const SharedGQSelector&, std::function<void(const GQNode* node)>) - Got trait with empty key. Skipping..." << std::endl;
					#endif
				#endif
				continue;
			}

			const std::vector< const GQNode* >* fromTrait = nullptr;

			if (traitsIt->second.size() == 0)
			{
				fromTrait = m_rootTreeMap->Get(GetUniqueId(), traitsIt->first);
			}
			else
			{
				fromTrait = m_rootTreeMap->Get(GetUniqueId(), traitsIt->first, traitsIt->second);
			}

			#ifndef NDEBUG
				#ifdef GQ_VERBOSE_DEBUG_NFO
					if (fromTrait != nullptr)
					{
						std::cout << u8"In GQNode::Each(const SharedGQSelector&, std::function<void(const GQNode* node)>) - Got " << fromTrait->size() << u8" candidates from trait." << std::endl;
					}
					else
					{
						std::cout << u8"In GQNode::Each(const SharedGQSelector&, std::function<void(const GQNode* node)>) - Got zero candidates from trait." << std::endl;
					}
				#endif
			#endif

			if (fromTrait != nullptr)
			{
				auto tSize = fromTrait->size();

				for (size_t i = 0; i < tSize; ++i)
				{
					// It's actually significantly faster to simply match then search for duplicates, rather than eliminate duplicates
					// first and then attempt a match.
					auto* pNode = (*fromTrait)[i];

					auto matchTest = selector->Match(pNode);
					if (matchTest)
					{
						auto matchedNode = matchTest.GetResult();

						if (collected.find(matchedNode->GetUniqueId()) == collected.end())
						{
							collected.insert({ matchedNode->GetUniqueId(), matchedNode->GetUniqueId() });
							func(matchedNode);
						}
					}
				}
			}
		}
	}

	const boost::string_ref GQNode::GetUniqueId() const
	{
		return boost::string_ref(m_nodeUniqueId);
	}

	std::string GQNode::GetInnerHtml() const
	{
		return GQSerializer::SerializeContent(this);
	}

	std::string GQNode::GetOuterHtml() const
	{
		return GQSerializer::Serialize(this);
	}

	void GQNode::BuildChildren()
	{
		auto numChildren = m_node->v.element.children.length;

		if (numChildren == 0)
		{
			return;
		}

		m_children.reserve(numChildren);

		size_t trueIndex = 0;

		for (size_t i = 0; i < numChildren; ++i)
		{
			const GumboNode* child = static_cast<GumboNode*>(m_node->v.element.children.data[i]);
			if (child->type != GUMBO_NODE_ELEMENT && child->type != GUMBO_NODE_TEMPLATE)
			{				
				continue;
			}

			auto sChild = GQNode::Create(child, m_rootTreeMap, m_nodeUniqueId, trueIndex, this);
			if (sChild != nullptr)
			{
				m_children.emplace_back(std::move(sChild));
				++trueIndex;
			}			
		}
	}

	void GQNode::BuildAttributes()
	{

		// Create an attribute map specifically for the GQTreeMap object. This is separate
		// from the unordered_map that we use for a local attribute map. The GQTreeMap::AttributeMap
		// object is a multimap, as we split whitespace separated attribute values into duplicate
		// key entries with different values.
		GQTreeMap::AttributeMap treeAttribMap;

		auto nodeTagName = GetTagName();

		// Push the node normalized tag name as an attribute
		treeAttribMap.insert({ GQSpecialTraits::GetTagKey(), nodeTagName });

		const GumboVector* attribs = &m_node->v.element.attributes;

		if (attribs != nullptr && attribs->length > 0)
		{
			for (size_t i = 0; i < attribs->length; ++i)
			{
				const GumboAttribute* attribute = static_cast<GumboAttribute*>(attribs->data[i]);

				boost::string_ref attribName;
				boost::string_ref attribValue;

				if (attribute->original_name.length > 0)
				{
					attribName = boost::string_ref(attribute->original_name.data, attribute->original_name.length);
				}

				if (attribute->original_value.length > 0)
				{
					attribValue = boost::string_ref(attribute->original_value.data, attribute->original_value.length);
				}

				GQUtil::TrimEnclosingQuotes(attribName);
				GQUtil::TrimEnclosingQuotes(attribValue);

				if (attribName.size() == 0)
				{
					continue;
				}

				m_attributes.insert({ attribName, attribValue });

				treeAttribMap.insert({ attribName, attribValue });

				// Split the attribute values up and store them individually
				auto anySplittablePos = attribValue.find(' ');

				if (anySplittablePos != boost::string_ref::npos)
				{
					bool splitOnce = false;
					while (anySplittablePos != boost::string_ref::npos && attribValue.size() > 0)
					{
						if (anySplittablePos > 0)
						{
							splitOnce = true;
							auto singleValue = attribValue.substr(0, anySplittablePos);
							treeAttribMap.insert({ attribName, singleValue });
						}

						attribValue = attribValue.substr(anySplittablePos + 1);
						anySplittablePos = attribValue.find(' ');
					}
					if (splitOnce && attribValue.size() > 0)
					{
						treeAttribMap.insert({ attribName, attribValue });
					}
				}
			}
		}		

		#ifndef GQ_FIND_NO_OP
		// Add the attributes to the tree map
		m_rootTreeMap->AddNodeToMap(GetUniqueId(), this, treeAttribMap);

		// Now we need to recursively append upwards. 
		for (GQNode* parent = GetParent(); parent != nullptr; parent = parent->GetParent())
		{
			auto parentScopeId = parent->GetUniqueId();

			m_rootTreeMap->AddNodeToMap(parentScopeId, this, treeAttribMap);
		}
		#endif
	}

} /* namespace gq */

