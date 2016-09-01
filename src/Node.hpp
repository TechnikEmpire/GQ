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
#include <functional>
#include <boost/utility/string_ref.hpp>
#include <boost/algorithm/string.hpp>
#include "Selector.hpp"
#include "StrRefHash.hpp"

namespace gq
{
	
	class Selection;
	class TreeMap;

	/// <summary>
	/// The Node class serves as a wrapper around a GumboNode raw pointer. It is not possible to
	/// construct a Node around a nullptr GumboNode* object, so if the Node is valid, then its
	/// internal GumboNode* is guaranteed to be valid. Furthermore, it's not possible for a Node
	/// to be created, except by the internals of this library, which again verifies the validity of
	/// underlying structures.
	///  <para>&#160;</para>
	/// There are a lot of raw pointers being provided, but these are managed in a tightly
	/// controlled fashion. If the user is provided access to a raw pointer, it should be assumed
	/// that the pointer is valid so long as the original Document unique_ptr is valid. The user
	/// should also not assume responsibility for the lifetime of a raw pointer received, only
	/// objects returned in smart pointer containers are given over to the user to manage.
	/// </summary>
	class Node
	{		

		/// <summary>
		/// In the original library, the CNode (Node now) was completely separate from everything
		/// else, but local instances were generated and copied all over the place whenever the user
		/// needed to search against a non-document node, or access a specific child. Also, these
		/// objects implemented a custom-rolled type of shared pointer. I didn't like this, so I
		/// opted to change this, managing them in standard smart pointers.
		/// <para>&#160;</para>
		/// As a result of this and some other changes, Node has a few friends.
		/// </summary>
		friend class Util;
		friend class Serializer;
		friend class NodeMutationCollection;

	public:	

		/// <summary>
		/// Default destructor.
		/// </summary>
		virtual ~Node();

		/// <summary>
		/// Gets the parent of this node. May be nullptr.
		/// </summary>
		/// <returns>
		/// A pointer to the parent Node, if a valid parent exists. If this
		/// node does not have a valid parent, return is nullptr.
		/// </returns>
		Node* GetParent() const;

		/// <summary>
		/// Gets the index of this node within its parent. This index differs from the actual index
		/// of the raw GumboNode->index_within_parent. Since Node/Document only stores nodes that
		/// are of type ELEMENT, returning GumboNode->index_within_parent would give the wrong index
		/// certainly as far as these containers are concerned.
		/// </summary>
		/// <returns>
		/// The index within the parent.
		/// </returns>
		const size_t GetIndexWithinParent() const;

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
		/// A valid and non-nullptr UniqueNode representing the child at the supplied index. 
		/// </returns>
		const Node* GetChildAt(const size_t index) const;

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
		/// such, we don't push things like gumbo text nodes as Nodes into the m_children
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
		const GumboTag GetTag() const;

		/// <summary>
		/// Run a selector against the node and its descendants and return any and all nodes that
		/// were matched by the supplied selector string. Note that this method, which accepts a
		/// selector as a string, internally calls the Parser::Parse(...) method, which will throw
		/// when supplied with invalid selectors. As such, be prepared to handle exceptions when
		/// using this method.
		/// <para>&#160;</para>
		/// Note also that it is recommended to use the Parser directly to compile selectors
		/// first, saving the returned SharedSelector objects. This is much more efficient if the
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
		const Selection Find(const std::string& selectorString) const;

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
		const Selection Find(const SharedSelector& selector) const;

		/// <summary>
		/// Runs a selector against the node and its descendants, and for each match found, invokes
		/// the supplied function with the matched node as the sole argument. This allows for the
		/// possibility of users to both find and perform operations on the matches in one
		/// iteration, rather then collecting matches and then iterating over them again.
		/// <para>&#160;</para>
		/// Note that this method, which accepts a selector as a string, internally calls the
		/// Parser::Parse(...) method, which will throw when supplied with invalid selectors. As
		/// such, be prepared to handle exceptions when using this method.
		/// <para>&#160;</para>
		/// Note also that it is recommended to use the Parser directly to compile selectors
		/// first, saving the returned SharedSelector objects. This is much more efficient if the
		/// selector is used more than once. Methods that accept raw selector strings will compile
		/// and discard selectors after use.
		/// </summary>
		/// <param name="selectorString">
		/// The selector string to query against the node and its descendants with. 
		/// </param>
		/// <param name="func">
		/// The callback that positive matches will be supplied to.
		/// </param>
		void Each(const std::string& selectorString, std::function<void(const Node* node)> func) const;

		/// <summary>
		/// Runs a selector against the node and its descendants, and for each match found, invokes
		/// the supplied function with the matched node as the sole argument. This allows for the
		/// possibility of users to both find and perform operations on the matches in one
		/// iteration, rather then collecting matches and then iterating over them again.
		/// </summary>
		/// <param name="selectorString">
		/// The precompiled selector object to query against the node and its descendants with. 
		/// </param>
		/// <param name="func">
		/// The callback that positive matches will be supplied to.
		/// </param>
		void Each(const SharedSelector& selector, std::function<void(const Node* node)> func) const;

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
		/// Interface to create a UniqueNode instance. In order to ensure the validity of
		/// structures and to maintain a proper, clear ownership model, new instances of Node can
		/// only be created through this interface by the library internals.
		/// </summary>
		/// <param name="node">
		/// The GumboNode* object that the Node is to wrap. This must be a valid pointer, or this
		/// method will throw.
		/// </param>
		/// <returns>
		/// A UniqueNode instance. 
		/// </returns>
		static std::unique_ptr<Node> Create(const GumboNode* node, TreeMap* map, const std::string& parentId, const size_t indexWithinParent = 0, Node* parent = nullptr);

		/// <summary>
		/// Empty constructor to satisfy Document.
		/// </summary>
		Node();

		/// <summary>
		/// Constructs a new node around a raw GumboNode pointer. 
		/// </summary>
		/// <param name="node">
		/// Pointer to a GumboNode. Must not be nullptr. 
		/// </param>
		/// <param name="newUniqueId">
		/// The generated unique ID for the new node. This must be completely constructed before
		/// being passed into this parameter.
		/// </param>
		/// <param name="indexWithinParent">
		/// The index within the parent. Node that this index is not necessarily equal to
		/// GumboNode::index_within_parent. This index rather, is the index when only
		/// GUMBO_NODE_ELEMENT objects are considered children. All Node objects are valid
		/// GUMBO_NODE_ELEMENTs. No other type of Gumbo element should be constructed into a Node
		/// object.
		/// </param>
		/// <param name="parent">
		/// Pointer to the parent GumboNode. Can be nullptr. 
		/// </param>
		Node(const GumboNode* node, const std::string newUniqueId, const size_t indexWithinParent, Node* parent);

		/// <summary>
		/// The raw GumboNode* that this object wraps.
		/// </summary>
		const GumboNode* m_node;

		/// <summary>
		/// A pointer to this node's parent, if any.
		/// </summary>
		Node* m_parent = nullptr;

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
		std::vector < std::unique_ptr<Node> > m_children;

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
							}
						}
						else
						{
							return std::memcmp(str.first.begin(), value.second.begin(), oneSize) == 0;
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
						}
					}
					else
					{
						return std::memcmp(str.first.begin(), key.begin(), oneSize) == 0;
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
		/// Populate the m_children element. Since this is invoked in the constructor, this will
		/// recursively build out all descendants of this node in the same way. We must ensure
		/// that nodes are only created from the Document, so that duplicate trees like this
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
		/// Object composition is getting a little ugly at this point. Document holds the single
		/// TreeMap in a unique_ptr as a private member, but Document was changed to inherit
		/// from Node. As such, when Document creates all of the Node objects for the
		/// document, it supplies the TreeMap* raw pointer, so we'll save the pointer to the tree
		/// map in a private member. Nodes need to reference the map when performing searches. This
		/// is a little ugly because Document is holding two pointers to the same thing, one
		/// itself and one inherited. Oh well.
		/// <para>&#160;</para>
		/// I'm not very fond of this, but the the user still only needs to keep Document alive,
		/// which is a very simple and reasonable requirement (to keep the document alive for so
		/// long as you're using it).
		/// </summary>
		TreeMap* m_rootTreeMap = nullptr;

	private:

			Node(const Node&) = delete;
			Node& operator=(const Node&) = delete;


			/// <summary>
			/// Holds node tag name as string so that it can be provided via the public interface
			/// which exposes it as a string_ref.
			/// </summary>
			std::string m_nodeTagNameString;
	};

	typedef std::unique_ptr<Node> UniqueNode;

} /* namespace gq */
