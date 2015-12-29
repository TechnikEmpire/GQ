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

#include "Node.hpp"
#include "Util.hpp"
#include "Selection.hpp"
#include "Parser.hpp"
#include "TreeMap.hpp"
#include "SpecialTraits.hpp"
#include "Serializer.hpp"

namespace gq
{

	std::unique_ptr<Node> Node::Create(const GumboNode* node, TreeMap* map, const std::string& parentId, const size_t indexWithinParent, Node* parent)
	{
		std::string newNodeId = parentId + std::string(u8"A") + std::to_string(indexWithinParent);
		auto newNode = std::unique_ptr<Node>{ new Node(node, newNodeId, indexWithinParent, parent) };

		#ifndef NDEBUG		
			assert(map != nullptr && u8"In Node::Create(const GumboNode*, TreeMap*, const std::string&, const size_t, Node*) - Cannot initialize a Node without a valid TreeMap* pointer. TreeMap* is nullptr.");
		#else		
			if (map == nullptr) { throw std::runtime_error(u8"In Node::Create(const GumboNode*, TreeMap*, const std::string&, const size_t, Node*) - Cannot initialize a Node without a valid TreeMap* pointer. TreeMap* is nullptr."); }
		#endif	

		newNode->m_rootTreeMap = map;

		newNode->BuildAttributes();
		newNode->BuildChildren();

		return newNode;
	}

	Node::Node()
	{
		m_parent = nullptr;
	}

	Node::Node(const GumboNode* node, const std::string newUniqueId, const size_t indexWithinParent, Node* parent) :
		m_node(node), 
		m_nodeUniqueId(std::move(newUniqueId)),
		m_indexWithinParent(indexWithinParent), 
		m_parent(parent)
	{		
		#ifndef NDEBUG
			assert(node != nullptr && u8"In Node::Node(const GumboNode*) - Cannot construct a Node around a nullptr.");		
		#else
			if (node == nullptr) { throw std::runtime_error(u8"In Node::Node(const GumboNode*) - Cannot construct a Node around a nullptr."); }		
		#endif			
	}	

	Node::~Node()
	{

	}

	Node* Node::GetParent() const
	{
		return m_parent;
	}

	const size_t Node::GetIndexWithinParent() const
	{
		return m_indexWithinParent;
	}

	const size_t Node::GetNumChildren() const
	{
		return m_children.size();
	}

	const Node* Node::GetChildAt(const size_t index) const
	{
		if (index >= m_children.size())
		{
			throw std::runtime_error(u8"In Node::GetChildAt(const size_t) - Supplied index is out of bounds.");
		}

		return m_children[index].get();
	}

	const bool Node::HasAttribute(const std::string& attributeName) const
	{
		boost::string_ref ref(attributeName);
		return HasAttribute(ref);
	}

	const bool Node::HasAttribute(const boost::string_ref attributeName) const
	{
		auto search = m_attributes.find(attributeName);
		return search != m_attributes.end();
	}

	const bool Node::IsEmpty() const
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

	boost::string_ref Node::GetAttributeValue(const boost::string_ref attributeName) const
	{			
		auto res = m_attributes.find(attributeName);
		if (res != m_attributes.end())
		{
			return res->second;
		}

		return boost::string_ref();
	}

	std::string Node::GetText() const
	{
		return Util::NodeText(this);
	}

	std::string Node::GetOwnText() const
	{
		return Util::NodeOwnText(this);
	}

	const size_t Node::GetStartPosition() const
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
				// XXX TODO - I don't like the idea of returning zero since it implies a valid
				// position. Perhaps change the return values to be int32_t and return -1 for
				// invalid nodes. However, matching shouldn't work against non element and non text
				// nodes, so there theoretically could never be a situation where this code is
				// reached. For now this is to shut up the compiler.
				return 0;
		}
	}

	const size_t Node::GetEndPosition() const
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
				// XXX TODO - I don't like the idea of returning zero since it implies a valid
				// position. Perhaps change the return values to be int32_t and return -1 for
				// invalid nodes. However, matching shouldn't work against non element and non text
				// nodes, so there theoretically could never be a situation where this code is
				// reached. For now this is to shut up the compiler.
				return 0;
		}
	}

	const size_t Node::GetStartOuterPosition() const
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
				// XXX TODO - I don't like the idea of returning zero since it implies a valid
				// position. Perhaps change the return values to be int32_t and return -1 for
				// invalid nodes. However, matching shouldn't work against non element and non text
				// nodes, so there theoretically could never be a situation where this code is
				// reached. For now this is to shut up the compiler.
				return 0;
		}
	}

	const size_t Node::GetEndOuterPosition() const
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
				// XXX TODO - I don't like the idea of returning zero since it implies a valid
				// position. Perhaps change the return values to be int32_t and return -1 for
				// invalid nodes. However, matching shouldn't work against non element and non text
				// nodes, so there theoretically could never be a situation where this code is
				// reached. For now this is to shut up the compiler.
				return 0;
		}
	}

	boost::string_ref Node::GetTagName() const
	{	
		return Util::GetNodeTagName(m_node);
	}

	const GumboTag Node::GetTag() const
	{
		return m_node->v.element.tag;
	}

	const Selection Node::Find(const std::string& selectorString) const
	{
		Parser parser;

		auto selector = parser.CreateSelector(selectorString);

		return Find(selector);
	}

	const Selection Node::Find(const SharedSelector& selector) const
	{

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_DEBUG_NFO
					std::cout << u8"Node::Find(const SharedSelector&)" << std::endl;
			#endif
		#endif

		std::vector<const Node*> matchResults;
		
		const auto& traits = selector->GetMatchTraits();

		// The collected map ensure that we don't store duplicate matches. Any time a match is made,
		// it's pushed to the collected map.
		FastAttributeMap collected;

		for (auto& traitsIt = traits.begin(); traitsIt != traits.end(); ++traitsIt)
		{

			#ifndef NDEBUG
				#ifdef GQ_VERBOSE_DEBUG_NFO
					std::cout << u8"In Node::Find(const SharedSelector&) - Finding potential matches at scope " << GetUniqueId() << u8" by trait: " << traitsIt->first << u8" ::: " << traitsIt->second << std::endl;
			#endif
			#endif

			if (traitsIt->first.size() == 0)
			{
				#ifndef NDEBUG
					#ifdef GQ_VERBOSE_DEBUG_NFO
							std::cout << u8"In Node::Find(const SharedSelector&) - Got trait with empty key. Skipping..." << std::endl;
					#endif
				#endif
				continue;
			}

			const std::vector< const Node* >* fromTrait = nullptr;

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
						std::cout << u8"In Node::Find(const SharedSelector&) - Got " << fromTrait->size() << u8" candidates from trait." << std::endl;
					}
					else
					{
						std::cout << u8"In Node::Find(const SharedSelector&) - Got zero candidates from trait." << std::endl;
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
					// It's actually significantly faster to simply match then search for
					// duplicates, rather than eliminate duplicates first and then attempt a match.
					const Node* pNode = (*fromTrait)[i];

					auto matchTest = selector->Match(pNode);
					if (matchTest)
					{
						auto* matchedNode = matchTest.GetResult();

						if (collected.find(matchedNode->GetUniqueId()) == collected.end())
						{							
							collected.insert({ matchedNode->GetUniqueId(), matchedNode->GetUniqueId() });
							matchResults.push_back(matchedNode);
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

		return Selection(matchResults);
	}

	void Node::Each(const std::string& selectorString, std::function<void(const Node* node)> func) const
	{
		Parser parser;

		auto selector = parser.CreateSelector(selectorString);

		Each(selector, func);
	}

	void Node::Each(const SharedSelector& selector, std::function<void(const Node* node)> func) const
	{
		const auto& traits = selector->GetMatchTraits();

		// The collected map ensures that we don't store duplicate matches. Any time a match is
		// made, it's pushed to the collected map.
		FastAttributeMap collected;

		for (auto& traitsIt = traits.begin(); traitsIt != traits.end(); ++traitsIt)
		{

			#ifndef NDEBUG
				#ifdef GQ_VERBOSE_DEBUG_NFO
					std::cout << u8"In Node::Each(const SharedSelector&, std::function<void(const Node* node)>) - Finding potential matches at scope " << GetUniqueId() << u8" by trait: " << traitsIt->first << u8" ::: " << traitsIt->second << std::endl;
				#endif
			#endif

			if (traitsIt->first.size() == 0)
			{
				#ifndef NDEBUG
					#ifdef GQ_VERBOSE_DEBUG_NFO
						std::cout << u8"In Node::Each(const SharedSelector&, std::function<void(const Node* node)>) - Got trait with empty key. Skipping..." << std::endl;
					#endif
				#endif
				continue;
			}

			const std::vector< const Node* >* fromTrait = nullptr;

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
						std::cout << u8"In Node::Each(const SharedSelector&, std::function<void(const Node* node)>) - Got " << fromTrait->size() << u8" candidates from trait." << std::endl;
					}
					else
					{
						std::cout << u8"In Node::Each(const SharedSelector&, std::function<void(const Node* node)>) - Got zero candidates from trait." << std::endl;
					}
				#endif
			#endif

			if (fromTrait != nullptr)
			{
				auto tSize = fromTrait->size();

				for (size_t i = 0; i < tSize; ++i)
				{
					// It's actually significantly faster to simply match then search for
					// duplicates, rather than eliminate duplicates first and then attempt a match.
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

	const boost::string_ref Node::GetUniqueId() const
	{
		return boost::string_ref(m_nodeUniqueId);
	}

	std::string Node::GetInnerHtml() const
	{
		return Serializer::SerializeContent(this);
	}

	std::string Node::GetOuterHtml() const
	{
		return Serializer::Serialize(this);
	}

	void Node::BuildChildren()
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

			auto sChild = Node::Create(child, m_rootTreeMap, m_nodeUniqueId, trueIndex, this);
			if (sChild != nullptr)
			{
				m_children.emplace_back(std::move(sChild));
				++trueIndex;
			}			
		}
	}

	void Node::BuildAttributes()
	{

		// Create an attribute map specifically for the TreeMap object. This is separate from the
		// unordered_map that we use for a local attribute map. The TreeMap::AttributeMap object
		// is a multimap, as we split whitespace separated attribute values into duplicate key
		// entries with different values.
		TreeMap::AttributeMap treeAttribMap;

		auto nodeTagName = GetTagName();

		// Push the node normalized tag name as an attribute
		treeAttribMap.insert({ SpecialTraits::GetTagKey(), nodeTagName });

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
				
				attribValue = Util::TrimEnclosingQuotes(attribValue);

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
		for (Node* parent = GetParent(); parent != nullptr; parent = parent->GetParent())
		{
			auto parentScopeId = parent->GetUniqueId();

			m_rootTreeMap->AddNodeToMap(parentScopeId, this, treeAttribMap);
		}
		#endif
	}

} /* namespace gq */

