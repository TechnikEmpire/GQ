/*
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

#include <stdexcept>
#include "TreeMap.hpp"
#include "Node.hpp"
#include "SpecialTraits.hpp"

namespace gq
{

	TreeMap::TreeMap()
	{

	}

	TreeMap::~TreeMap()
	{

	}	

	void TreeMap::AddNodeToMap(boost::string_ref scope, const Node* node, const AttributeMap& nodeAttributeMap)
	{
		#ifndef NDEBUG
			assert(scope.size() > 0 && u8"In QTreeMap::AddNodeToMap(boost::string_ref, const Node*, const AttributeMap&) - The supplied scope is empty. This error is impossible unless a user is directly and incorrectly calling this method, or if this class and its required mechanisms are fundamentally broken.");
		#else
			if (scope.size() == 0) { throw std::runtime_error(u8"In QTreeMap::AddNodeToMap(boost::string_ref, const Node*, const AttributeMap&) - The supplied scope is empty. This error is impossible unless a user is directly and incorrectly calling this method, or if this class and its required mechanisms are fundamentally broken."); }
		#endif
		
		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_DEBUG_NFO					
				std::cout << u8"Adding node " << node->GetUniqueId() << u8" to scope " << scope << u8" with attributes:" << std::endl;
				for (auto& eachAttr = nodeAttributeMap.begin(); eachAttr != nodeAttributeMap.end(); ++eachAttr)
				{
					std::cout << u8"\tName: " << eachAttr->first << u8" ::: Value: " << eachAttr->second << std::endl;
				}
				std::cout << std::endl;
			#endif
		#endif

		auto& it = m_scopedAttributes.find(scope);

		CollectedAttributesMap* col;

		if (it == m_scopedAttributes.end())
		{
			// Insert a new attribute map for the given scope if it doesn't exists already.
			auto& res = m_scopedAttributes.emplace(std::make_pair(scope, CollectedAttributesMap{}));
			col = &res.first->second;
		}
		else
		{
			col = &it->second;
		}		

		for (auto& nodeAttrMapIt = nodeAttributeMap.begin(); nodeAttrMapIt != nodeAttributeMap.end(); ++nodeAttrMapIt)
		{
			// Need to find the attribute key at this scope. If it doesn't exist, create it and push values. If it
			// does exist, need to append the node. Must append the node both with the value as the key, and as
			// "*" as the key (for EXISTS lookups).	

			auto& attr = col->find(nodeAttrMapIt->first);

			if (attr == col->end())
			{
				// Add the node with "*" as the value key. This is useful for EXISTS lookups, prefix/suffix/list matching,
				// etc. The node has the attribute being sought, that's all the user cares about.
				auto newCont1 = std::vector< const Node* >{ { node } };
				auto newMap1 = ValueToNodesMap{};
				newMap1.emplace(std::make_pair(SpecialTraits::GetAnyValue(), std::move(newCont1)));			
				
				if (nodeAttrMapIt->second.size() > 0)
				{
					auto newCont2 = std::vector< const Node* >{ { node } };

					// If the attribute is more than EXISTS, and defines a value, push it here as well.
					newMap1.emplace(std::make_pair(nodeAttrMapIt->second, std::move(newCont2)));
				}	

				col->emplace(std::make_pair(nodeAttrMapIt->first, std::move(newMap1)));
			}
			else
			{
				// There is already an entry for the attribute name, but that just means a map exists. Have to
				// check if values exists and if so, append, if not, insert/create the key and vector collection.

				auto& anyValueMatch = attr->second.find(SpecialTraits::GetAnyValue());
				auto& exactValueMatch = attr->second.find(nodeAttrMapIt->second);

				// We need to check to make sure that the node doesn't already exist in the collection. This is because
				// of the way that we split up space and hyphen delimited lists in attribute values. For example, when
				// constructing the attribute map for a node with the attribute 'class="one two three"', we'll push
				// the same attribute "class" into a multimap with three entries: '{"class", "one"}' '{"class", "two"}'
				// so on and so forth. We don't want to push the same node to the collection three times, this is 
				// wasteful. The node simply needs to be found once for any one of those searches.
				//
				// We search for equality of the raw node pointer.
				//
				// Also XXX TODO - Methinks we shouldn't be splitting up attributes separated by hypen at all. The
				// original purpose was to optimize the AttributeSelector::Match(...) method when doing matching
				// against hyphen separated lists. However, I think this is invalid and will lead to conflicts,
				// since the hyphen matching simply functions like a begins-with match. If this is the case and
				// we stop doing that, we don't need to make any changes here. Just in Node::BuildAttributes(...).
				//
				// Update - no longer splitting by "-" character, only whitespace in attributes.
				if (anyValueMatch != attr->second.end())
				{
					if (std::find_if(anyValueMatch->second.begin(), anyValueMatch->second.end(),
						[node](const Node* elm)
					{
						return elm == node;
					}) == anyValueMatch->second.end())
					{
						anyValueMatch->second.push_back(node);
					}					
				}
				else
				{
					// No need to search for duplicates, since no entries exist.
					auto cont = std::vector< const Node* >{ { node } };
					attr->second.emplace(std::make_pair(SpecialTraits::GetAnyValue(), std::move(cont)));
				}

				if (exactValueMatch != attr->second.end())
				{
					// There should be no need to search for duplicates when indexing by exact
					// attribute name and exact attribute value. There shouldn't be two attributes
					// with the exact same name and value. The only case where the possibility of
					// duplicates exists is pushing special attributes such as tag name, but because
					// the tree is built from a document recursively in order and is controlled (hidden
					// from the user), so long as there are no bugs with this, this itself should 
					// not produce duplicates either.
					exactValueMatch->second.push_back(node);
				}
				else
				{
					// No need to search for duplicates, since no entries exist.
					auto cont = std::vector< const Node* >{ { node } };
					attr->second.emplace(std::make_pair(nodeAttrMapIt->second, std::move(cont)));
				}
			}
		}		
	}

	const std::vector< const Node* >* TreeMap::Get(boost::string_ref scope, boost::string_ref attribute) const
	{
		return Get(scope, attribute, SpecialTraits::GetAnyValue());
	}

	const std::vector< const Node* >* TreeMap::Get(boost::string_ref scope, boost::string_ref attribute, boost::string_ref attributeValue) const
	{
		// First, jump the the correct scope to begin matching from.
		auto& atScope = m_scopedAttributes.find(scope);

		#ifndef NDEBUG
			assert(atScope != m_scopedAttributes.end() && u8"In TreeMap::Get(boost::string_ref, boost::string_ref, boost::string_ref) - The supplied scope does not exist. This error is impossible unless a user is directly and incorrectly calling this method, or if this class and its required mechanisms are fundamentally broken.");
		#else
			if (atScope == m_scopedAttributes.end())
			{
				// This should not be possible, provided users are messing about and the
				// implementation isn't fundamentally broken.
				throw std::runtime_error(u8"In TreeMap::Get(boost::string_ref, boost::string_ref, boost::string_ref) - The supplied scope does not exist. This error is impossible unless a user is directly and incorrectly calling this method, or if this class and its required mechanisms are fundamentally broken.");
			}
		#endif		

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_DEBUG_NFO
				std::cout << u8"In TreeMap::Get(boost::string_ref, boost::string_ref, boost::string_ref) - Looking up at scope " << scope << u8" with key " << attribute << u8" and value " << attributeValue << u8"." << std::endl;
			#endif
		#endif

		// Search for the attribute name at this scope		
		auto& byAttrName = atScope->second.find(attribute);
		if (byAttrName != atScope->second.end())
		{
			// If we found matches to the attribute, search for the exact value
			auto& byAttrValue = byAttrName->second.find(attributeValue);
						
			if (byAttrValue != byAttrName->second.end())
			{
				// If we have matched both attribute and value, return the corresponding collection.
				return &byAttrValue->second;
			}
		}
		
		return nullptr;
	}

	void TreeMap::Clear()
	{
		m_scopedAttributes.clear();
	}

} /* namespace gq */
