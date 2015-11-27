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

#include <boost/utility/string_ref.hpp>
#include <memory>
#include <boost/algorithm/string.hpp>
#include "GQSelector.hpp"

namespace gumboquery
{
	
	class GQSelection;

	/// <summary>
	/// The GQNode class serves as a wrapper around a GumboNode raw pointer. It is not possible to
	/// construct a GQNode around a nullptr GumboNode* object, so if the GQNode is valid, then its
	/// internal GumboNode* is guaranteed to be valid.
	/// 
	/// Note that since GumboNode objects are owned exclusively by the GumboOutput root object, this
	/// wrapper does not managed the lifetime of the raw pointer it matches. Users still need to
	/// take care that manually generated GumboOutput is destroyed correctly.
	/// </summary>
	class GQNode : public std::enable_shared_from_this<GQNode>
	{

		/// <summary>
		/// So, I don't really like the idea of this friend business and having to forward declare
		/// types to resolve circular dependencies, to me that means there's probably a better way.
		/// Unfortunately, I'm trying to work within design that I started out with.
		/// 
		/// In the original library, the CNode (GQNode now) was completely separate from everything
		/// else, but generated on the fly whenever the user needed to search against a non-document
		/// node, or access certain information. I didn't like this, generating potentially endless
		/// temporaries on every access. So, I opted to change this, making them shared.
		/// 
		/// This then forced the design all the way back down the library into complementary
		/// selection code which stored matched nodes in collections, vectors. In order to make the
		/// new design consistent, these all had to be changed to hold shared_ptrs of GQNode, not
		/// raw GumboNode pointers. Hence, we have forward declarations and friend declarations
		/// between GQUtil and GQSelection to allow this design. These two friends need to be able
		/// to access the raw GumboNode pointers privately held in GQNode.
		/// </summary>
		friend class GQSelection;
		friend class GQUtil;

	public:

		/// <summary>
		/// Constructs a new node around a raw GumboNode pointer.
		/// </summary>
		/// <param name="node">
		/// Pointer to a GumboNode. Default is nullptr.
		/// </param>
		GQNode(const GumboNode* node);

		/// <summary>
		/// Default destructor.
		/// </summary>
		~GQNode();

		/// <summary>
		/// Gets the parent of this node. May be nullptr.
		/// </summary>
		/// <returns>
		/// A shared GQNode that wraps the parent of this node, if a valid parent exists. If this
		/// node does not have a valid parent, a node is returned but its ::IsValid() member will
		/// report false.
		/// </returns>
		std::shared_ptr<GQNode> GetParent() const;

		/// <summary>
		/// Gets the index of this node within its parent. If the node is internally invalid or has
		/// no parent, the result is -1. In all other circumstances, the value is a positive value
		/// representing the true index.
		/// </summary>
		/// <returns>
		/// The index within the parent. If the node is internally invalid or the node has no
		/// parent, the return value will be -1. In all other circumstances, the return value will
		/// be the true index of this node within its parent.
		/// </returns>
		const size_t GetIndexWithinParent() const;

		/// <summary>
		/// Get the previous sibling. May be nullptr if the node does not have a previous sibling.
		/// </summary>
		/// <returns>
		/// A SharedGQNode which may or may not be valid. If its ::IsValid() member reports true,
		/// then the returned object indeed represents the previous sibling. If not, then it is a
		/// dummy object.
		/// </returns>
		std::shared_ptr<GQNode> GetPreviousSibling() const;

		/// <summary>
		/// Get the next sibling. May be nullptr if the node does not have a next sibling.
		/// </summary>
		/// <returns>
		/// A SharedGQNode which may or may not be valid. If its ::IsValid() member reports true,
		/// then the returned object indeed represents the next sibling. If not, then it is a dummy
		/// object.
		/// </returns>
		std::shared_ptr<GQNode> GetNextSibling() const;

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
		/// <param name="position">
		/// The position of the child to retrieve. 
		/// </param>
		/// <returns>
		/// A valid and non-nullptr SharedGQNode representing the child at the supplied index. 
		/// </returns>
		std::shared_ptr<GQNode> GetChildAt(const size_t position) const;

		/// <summary>
		/// Gets the value of the supplied attribute name. If the supplied named attribute is not
		/// found, or no value is defined, an empty string will be returned.
		/// </summary>
		/// <param name="attributeName">
		/// The named attribute to return the value of. 
		/// </param>
		/// <returns>
		/// A string which may be empty if the supplied named attribute was not found or contained
		/// no value.
		/// </returns>
		std::string GetAttributeValue(const std::string& attributeName) const;

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
		boost::string_ref GetAttributeValue(const boost::string_ref& attributeName) const;

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
		std::string GetTagName() const;

		/// <summary>
		/// Gets the tag of the node.
		/// </summary>
		/// <returns>
		/// The tag of the node.
		/// </returns>
		GumboTag GetTag() const;

		/// <summary>
		/// Queries this node and its descendants using the supplied selector string. Note that this
		/// method, which accepts a selector as a string, internally calls the GQParser::Parse(...)
		/// method, which will throw when supplied with invalid selectors. As such, be prepared to
		/// handle exceptions when using this method.
		/// 
		/// Note also that it is recommended to use the GQParser directly to compile selectors
		/// first, saving the returned GQSharedSelector objects. This is much more efficient if the
		/// selector is used more than once. Methods that accept raw selector strings will compile
		/// and discard selectors after use.
		/// </summary>
		/// <param name="selector">
		/// The selector string to compile and query against this node and its descendants for
		/// matches.
		/// </param>
		/// <returns>
		/// A GQSelection object that holds any and all matches found using the supplied selector
		/// string.
		/// </returns>
		GQSelection Find(const std::string& selector);

		/// <summary>
		/// Queries this node and its descendants using the supplied selector string. Note that
		/// unlike the overload that allows a string argument, this version cannot potentially
		/// throw. This overload is the recommended overload to use, because it recycles previously
		/// compiled selectors rather than compiling them and discarding them on the fly.
		/// </summary>
		/// <param name="selector">
		/// The compiled selector to query against this node and its descendants for matches. 
		/// </param>
		/// <returns>
		/// A GQSelection object that holds any and all matches found using the supplied selector
		/// string.
		/// </returns>
		GQSelection Find(const SharedGQSelector& selector);

	private:

		/// <summary>
		/// The raw GumboNode* that this object wraps.
		/// </summary>
		const GumboNode* m_node;

		/// <summary>
		/// A SharedGQNode that wraps the parent node. May be nullptr if this node has no parent.
		/// </summary>
		std::shared_ptr<GQNode> m_sharedParent;

		/// <summary>
		/// A SharedGQNode that wraps the immediate previous sibling of this node. If the parent
		/// node is nullptr, or this node is an only child or this node is at position zero within
		/// the parent, it will be nullptr.
		/// </summary>
		std::shared_ptr<GQNode> m_sharedPreviousSibling;

		/// <summary>
		/// A SharedGQNode that wraps the immediate next sibling of this node. If the parent node is
		/// nullptr, or this node is an only child or this node is the last child within the parent,
		/// it will be nullptr.
		/// </summary>
		std::shared_ptr<GQNode> m_sharedNextSibling;

	};

	typedef std::shared_ptr<GQNode> SharedGQNode;

} /* namespace gumboquery */
