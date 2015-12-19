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

#pragma once

#include <memory>
#include <map>
#include <boost/utility/string_ref.hpp>
#include <boost/algorithm/string.hpp>
#include "GQSelector.hpp"
#include "GQStrRefHash.hpp"

namespace gq
{
	
	class GQSelection;
	class GQTreeMap;

	/// <summary>
	/// The GQNode class serves as a wrapper around a GumboNode raw pointer. It is not possible to
	/// construct a GQNode around a nullptr GumboNode* object, so if the GQNode is valid, then its
	/// internal GumboNode* is guaranteed to be valid.
	/// <para>&#160;</para>
	/// Note that since GumboNode objects are owned exclusively by the GumboOutput root object, this
	/// wrapper does not manage the lifetime of the raw pointer it matches. Users still need to
	/// take care that manually generated GumboOutput is destroyed correctly.
	/// </summary>
	class GQNode : public std::enable_shared_from_this<GQNode>
	{

		// If MSVC, must friend this nonsense so that make_shared can access the private constructor
		// of our class. If not, just friend the template function.
		#ifdef _MSC_VER
		friend std::_Ref_count_obj<GQNode>;
		#else
		friend std::shared_ptr<GQNode> std::make_shared<>(const GumboNode*);
		#endif
		

		/// <summary>
		/// So, I don't really like the idea of this friend business and having to forward declare
		/// types to resolve circular dependencies, to me that means there's probably a better way.
		/// Unfortunately, I'm trying to work within design that I started out with.
		/// <para>&#160;</para>
		/// In the original library, the CNode (GQNode now) was completely separate from everything
		/// else, but local instances were generated and copied all over the place whenever the user
		/// needed to search against a non-document node, or access a specific child. I didn't like
		/// this, so I opted to change this, making them shared.
		/// <para>&#160;</para>
		/// This then forced the design all the way back down the library into complementary
		/// selection code which stored matched nodes in collections, vectors. In order to make the
		/// new design consistent, these all had to be changed to hold shared_ptrs of GQNode, not
		/// raw GumboNode pointers. Hence, we have forward declarations and friend declarations
		/// between GQUtil, GQSelection and GQSelector to allow this design. These three friends need
		/// to be able to access the raw GumboNode pointers privately held in GQNode. This could be
		/// all solved by making a public accessor to the internal raw GumboNode, but not presently
		/// convinced this approach is worse.
		/// </summary>
		friend class GQSelection;
		friend class GQUtil;
		friend class GQSelector;
		friend class GQTreeIndex;
		friend class GQSerializer;

	public:		

		/// <summary>
		/// Default destructor.
		/// </summary>
		virtual ~GQNode();

		/// <summary>
		/// Gets the parent of this node. May be nullptr.
		/// </summary>
		/// <returns>
		/// A shared GQNode that wraps the parent of this node, if a valid parent exists. If this
		/// node does not have a valid parent, a node is returned but its ::IsValid() member will
		/// report false.
		/// </returns>
		GQNode* GetParent() const;

		/// <summary>
		/// Gets the index of this node within its parent. This index differs from the actual index
		/// of the raw GumboNode->index_within_parent. Since GQNode/Document only stores nodes that
		/// are of type ELEMENT, returning GumboNode->index_within_parent would give the wrong index
		/// certainly as far as these containers are concerned.
		/// </summary>
		/// <returns>
		/// The index within the parent.
		/// </returns>
		const size_t& GetIndexWithinParent() const;

		/// <summary>
		/// Gets the number of children this node has.
		/// </summary>
		/// <returns>
		/// The total number of child nodes this node contains.
		/// </returns>
		const size_t GetNumChildren() const;

		/// <summary>
		/// Gets the child at the specified index. Note that this function will throw if the
		/// supplied index is out of bounds. Users should ensure that ::GetNumChildren() reports a
		/// value greater than zero, and that supplied indices are less than the number of children
		/// returned (zero based index).
		/// </summary>
		/// <param name="index">
		/// The index of the child to retrieve. 
		/// </param>
		/// <returns>
		/// A valid and non-nullptr SharedGQNode representing the child at the supplied index. 
		/// </returns>
		std::shared_ptr<GQNode> GetChildAt(const size_t index) const;

		/// <summary>
		/// Checks if the attribute by the name given exists for this node. Does not support prefix
		/// searching.
		/// </summary>
		/// <param name="attributeName">
		/// The name of the attribute to check. 
		/// </param>
		/// <returns>
		/// True if the attribute exists, false otherwise. 
		/// </returns>
		const bool HasAttribute(const std::string& attributeName) const;

		/// <summary>
		/// Overload that takes the named attribute by boost::string_ref const reference and returns
		/// whether or not the named attribute exists on this element. Does not support prefix
		/// searching.
		/// </summary>
		/// <param name="attributeName">
		/// The name of the attribute to check. 
		/// </param>
		/// <returns>
		/// True if the attribute exists, false otherwise. 
		/// </returns>
		const bool HasAttribute(const boost::string_ref attributeName) const;

		/// <summary>
		/// Check if the node is actually completely empty, or if it contains non-html element
		/// children. This is necessary for the :empty pseudo class selector because it's sort of a
		/// special case. For all other purposes, we don't care about non-html entity children. As
		/// such, we don't push things like gumbo text nodes as GQNodes into the m_children
		/// container. But, the :empty pseudo class selector needs to know about these things.
		/// </summary>
		/// <returns>
		/// True if the object contains absolutely no children of any kind, even text nodes, false
		/// otherwise.
		/// </returns>
		const bool IsEmpty() const;

		/// <summary>
		/// Overload that takes the named attribute by boost::string_ref const reference and returns
		/// the value in a boost::string_ref object. The purpose of this overload is to allow
		/// attribute lookup and comparison without making any copies of string data.
		/// </summary>
		/// <param name="attributeName">
		/// The named attribute to return the value of. 
		/// </param>
		/// <returns>
		/// A boost::string_ref which may be empty if the supplied named attribute was not found or
		/// contained no value.
		/// </returns>
		boost::string_ref GetAttributeValue(const boost::string_ref attributeName) const;

		/// <summary>
		/// Gets the text of this node and all of its text descendants combined. 
		/// </summary>
		/// <returns>
		/// A string which may be empty if this node is not a text node and none of its descendants
		/// are either. Otherwise, the string will be populated will the content of every text node
		/// from this node down through its descendants.
		/// </returns>
		std::string GetText() const;

		/// <summary>
		/// Gets the text of only the children of this node. 
		/// </summary>
		/// <returns>
		/// A string which may be empty if none of this nodes children are text nodes. Otherwise,
		/// the string will be populated will the content of every text node from this node down
		/// through its descendants.
		/// </returns>
		std::string GetOwnText() const;

		/// <summary>
		/// Gets the starting position of the contents of the node within the original HTML.
		/// </summary>
		/// <returns>
		/// The starting position of the contents of the node within the original HTML.
		/// </returns>
		const size_t GetStartPosition() const;

		/// <summary>
		/// Gets the ending position of the contents of the node within the original HTML.
		/// </summary>
		/// <returns>
		/// The ending position of the contents of the node within the original HTML.
		/// </returns>
		const size_t GetEndPosition() const;

		/// <summary>
		/// Gets the starting position of the node and its contents, including the node tag, in the
		/// original HTML.
		/// </summary>
		/// <returns>
		/// The starting position of the node and its contents, including the node tag, in the
		/// original HTML.
		/// </returns>
		const size_t GetStartOuterPosition() const;

		/// <summary>
		/// Gets the ending position of the node and its contents, including the node closing tag,
		/// in the original HTML.
		/// </summary>
		/// <returns>
		/// The starting position of the node and its contents, including the node closing tag, in
		/// the original HTML.
		/// </returns>
		const size_t GetEndOuterPosition() const;

		/// <summary>
		/// Gets the tag of the node as a string.
		/// </summary>
		/// <returns>
		/// The tag of the node as a string.
		/// </returns>
		boost::string_ref GetTagName() const;

		/// <summary>
		/// Gets the tag of the node.
		/// </summary>
		/// <returns>
		/// The tag of the node.
		/// </returns>
		const GumboTag& GetTag() const;

		/// <summary>
		/// Run a selector against the node and its descendants and return any and all nodes that
		/// were matched by the supplied selector string. Note that this method, which accepts a
		/// selector as a string, internally calls the GQParser::Parse(...) method, which will throw
		/// when supplied with invalid selectors. As such, be prepared to handle exceptions when
		/// using this method.
		/// <para>&#160;</para>
		/// Note also that it is recommended to use the GQParser directly to compile selectors
		/// first, saving the returned GQSharedSelector objects. This is much more efficient if the
		/// selector is used more than once. Methods that accept raw selector strings will compile
		/// and discard selectors after use.
		/// </summary>
		/// <param name="selectorString">
		/// The selector string to query against the node and its descendants with. 
		/// </param>
		/// <returns>
		/// A collection of nodes that were matched by the supplied selector. If no matches were
		/// found, the collection will be empty.
		/// </returns>
		const GQSelection Find(const std::string& selectorString) const;

		/// <summary>
		/// Run a selector against the node and its descendants and return any and all nodes that
		/// were matched by the supplied compiled selector.
		/// </summary>
		/// <param name="selector">
		/// The precompiled selector object to query against the node and its descendants with. 
		/// </param>
		/// <returns>
		/// A collection of nodes that were matched by the supplied selector. If no matches were
		/// found, the collection will be empty.
		/// </returns>
		const GQSelection Find(const SharedGQSelector& selector) const;

		/// <summary>
		/// Gets the unique ID of the node. See nodes on m_nodeUniqueId for more.
		/// </summary>
		/// <returns>
		/// The unique ID of the node.
		/// </returns>
		const boost::string_ref GetUniqueId() const;

		/// <summary>
		/// Gets the inner HTML for the node and its descendants in string format.
		/// </summary>
		/// <returns>
		/// The inner HTML for the node and its descendants in string format.
		/// </returns>
		std::string GetInnerHtml() const;

		/// <summary>
		/// Gets the outer HTML for the node and its descendants in string format.
		/// </summary>
		/// <returns>
		/// The outer HTML for the node and its descendants in string format.
		/// </returns>
		std::string GetOuterHtml() const;

	protected:

		/// <summary>
		/// Interface to create a SharedGQNode instance. Since GQNode inherits from
		/// enabled_shared_from_this and calls shared_from_this internally for some matching
		/// methods, its necessary to enforce that an object of this type cannot exist but already
		/// wrapped in a shared_ptr.
		/// </summary>
		/// <param name="node">
		/// The GumboNode* object that the GQNode is to wrap. This must be a valid pointer, or this
		/// method will throw.
		/// </param>
		/// <returns>
		/// A SharedGQNode instance. 
		/// </returns>
		static std::shared_ptr<GQNode> Create(const GumboNode* node, GQTreeMap* map, const std::string& parentId, const size_t indexWithinParent = 0, GQNode* parent = nullptr);

		/// <summary>
		/// Empty constructor to satisfy GQDocument.
		/// </summary>
		GQNode();

		/// <summary>
		/// Constructs a new node around a raw GumboNode pointer.
		/// </summary>
		/// <param name="node">
		/// Pointer to a GumboNode. Default is nullptr.
		/// </param>
		GQNode(const GumboNode* node, const std::string& parentId, const size_t indexWithinParent, GQNode* parent);

		/// <summary>
		/// The raw GumboNode* that this object wraps.
		/// </summary>
		const GumboNode* m_node;

		/// <summary>
		/// A pointer to this node's parent, if any.
		/// </summary>
		GQNode* m_parent = nullptr;

		/// <summary>
		/// Since we're excluding nodes that are not element nodes, we can't rely on the index value
		/// specified in GumboNode->index_within_parent. We have to generate and save our own proper
		/// index.
		/// </summary>
		size_t m_indexWithinParent;

		/// <summary>
		/// A unique ID for the node composed of its position within parent, and its parent's position
		/// within their parents all the way back to the root. If we were using integers for this, it
		/// simply wouldn't work (could never be unique), but by storing as string, this is possible.
		/// </summary>
		std::string m_nodeUniqueId;

		/// <summary>
		/// Container holding all valid html elements that are children of this html element.
		/// </summary>
		std::vector < std::shared_ptr<GQNode> > m_children;

		struct StringRefComparer : public std::binary_function<std::string,
			std::string, bool>
		{
			bool operator()(const boost::string_ref strOne, const boost::string_ref strTwo) const
			{
				auto oneSize = strOne.size();
				auto twoSize = strTwo.size();
				
				if (oneSize == twoSize)
				{
					if ((strOne[0] == strTwo[0]) && (strOne[strTwo.length() - 1] == strTwo[strTwo.length() - 1]) && (strOne[strTwo.length() - 2] == strTwo[strTwo.length() - 2]))
					{
						return strOne.compare(strTwo) == 0;
					}
				}

				return false;
			}
		};

		/// <summary>
		/// This is about 25 percent faster than using an unordered_map or map. Too great of a performance
		/// increase to pass up.
		/// </summary>
		struct FastAttributeMap
		{

		public:

			FastAttributeMap()
			{

			}

			std::vector<std::pair<boost::string_ref, boost::string_ref>>::const_iterator begin() const
			{								
				return m_collection.begin();
			}

			std::vector<std::pair<boost::string_ref, boost::string_ref>>::const_iterator end() const
			{
				return m_collection.end();
			}

			void insert(std::pair<boost::string_ref, boost::string_ref> value)
			{
				if (std::find_if(m_collection.begin(), m_collection.end(),
					[value](const std::pair<boost::string_ref, boost::string_ref>& str)-> bool
					{
						auto oneSize = str.first.size();
						auto twoSize = value.first.size();
						
						if (oneSize != twoSize)
						{
							return false;
						}

						if (oneSize >= 4)
						{
							if ((str.first[0] == value.first[0]) && 
								(str.first[1] == value.first[1]) && 
								(str.first[oneSize - 1] == value.first[oneSize - 1]) && 
								(str.first[oneSize - 2] == value.first[oneSize - 2]))
							{
								return std::memcmp(str.first.begin(), value.second.begin(), oneSize) == 0;
								//return str.first.compare(value.second) == 0;
							}
						}
						else
						{
							return std::memcmp(str.first.begin(), value.second.begin(), oneSize) == 0;
							//return str.first.compare(value.second) == 0;
						}

						return false;
					}
				) == m_collection.end())
				{
					m_collection.emplace_back(std::move(value));
				}
			}

			std::vector<std::pair<boost::string_ref, boost::string_ref>>::const_iterator find(const boost::string_ref key) const
			{
				return std::find_if(m_collection.begin(), m_collection.end(),
					[key](const std::pair<boost::string_ref, boost::string_ref>& str)-> bool
				{
					auto oneSize = str.first.size();
					auto twoSize = key.size();

					if (oneSize != twoSize)
					{
						return false;
					}

					if (oneSize >= 4)
					{
						if ((str.first[0] == key[0]) && 
							(str.first[1] == key[1]) && 
							(str.first[oneSize - 1] == key[oneSize - 1]) && 
							(str.first[oneSize - 2] == key[oneSize - 2]))
						{
							return std::memcmp(str.first.begin(), key.begin(), oneSize) == 0;
							//return str.first.compare(key) == 0;
						}
					}
					else
					{
						return std::memcmp(str.first.begin(), key.begin(), oneSize) == 0;
						//return str.first.compare(key) == 0;
					}

					return false;
				}
				);
			}

		private:

			std::vector<std::pair<boost::string_ref, boost::string_ref>> m_collection;
		};

		/// <summary>
		/// Holds preprocessed attributes for quick lookup. The attribute values in this member are
		/// untouched except for the fact that any enclosing quotes are pre-trimmed from the value
		/// string. This container differs from the m_allAttributeValues member in that it simply
		/// stores all instances of attribute/value. For example, consider the attribute "class='one
		/// two three'". This member, when "class" us used as the key, will return a single value of
		/// "one two three", without the quotes.
		/// <para>&#160;</para>
		/// The m_allAttributeValues member on the other hand would return an unordered set for the
		/// same key, containing individual entries of "one" "two" "three".
		/// </summary>
		FastAttributeMap m_attributes;

		/// <summary>
		/// Since we have all of this crazyness with SharedGQNode sprinkled everywhere, and due to
		/// the nature of the GQTreeMap and how we must work with it, an Init() member is required
		/// which must be called post-object construction. The GQTreeMap holds shared_ptr's to nodes
		/// for quick lookup based on attributes. When a GQNode is constructed, it needs to build
		/// its attributes and give it to the GQTreeMap. We simply cannot called shared_from_this()
		/// within the constructor, because the shared_ptr is not yet fully constructed at that
		/// point. As such, we need this.
		/// </summary>
		virtual void Init();

		/// <summary>
		/// Populate the m_children element. Since this is invoked in the constructor, this will
		/// recursively build out all descendants of this node in the same way. We must ensure
		/// that nodes are only created from the GQDocument, so that duplicate trees like this
		/// are not build, just for the sake of not wastefully duplicating.
		/// </summary>
		void BuildChildren();

		/// <summary>
		/// Extracts the attributes, if any, for this element, processes them by trimming enclosing
		/// quotes, and then populates the m_attributes unorder_map with values. Uses string_ref,
		/// so the values are not copied anywhere.
		/// </summary>
		void BuildAttributes();

		/// <summary>
		/// Object composition is getting pretty ugly at this point. GQDocument holds the single
		/// GQTreeMap in a unique_ptr as a private member. GQNode can't hold a shared_ptr to such an
		/// object because the GQTreeMap holds shared_ptr's to nodes that use it. This would create
		/// a circular reference, leading to memory leaks. Same reason that m_parent must be a raw
		/// pointer. GQDocument is also a subclass of of GQNode, so we can't just hold a pointer to
		/// the document root and access it that way. As such, when GQDocument creates all of the
		/// GQNode objects for the document, it supplies the GQTreeMap* raw pointer, so we'll save
		/// the pointer to the tree map in a private member. Nodes need to reference the map when
		/// performing searches.
		/// <para>&#160;</para>
		/// As ugly (confusing) as this is getting internally, the user still only needs to keep
		/// GQDocument alive, which is a very simple and reasonable requirement (to keep the
		/// document alive for so long as you're using it). In the event that the user honors this
		/// contract, m_rootTreeMap should never end up being null, so this should be reliable.
		/// </summary>
		GQTreeMap* m_rootTreeMap = nullptr;

	};

	typedef std::shared_ptr<GQNode> SharedGQNode;

} /* namespace gq */
