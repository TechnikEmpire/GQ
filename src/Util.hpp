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

#include <gumbo.h>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <boost/utility/string_ref.hpp>

namespace gq
{	
	
	class Node;

	/// <summary>
	/// Util is, as the name suggested, a Utility class containing behavior that is commonly
	/// beneficial across all classes.
	/// </summary>
	class Util
	{

	public:		

		/// <summary>
		/// Get the text of the supplied node and all of its descendants. 
		/// </summary>
		/// <param name="node">
		/// The node from which to extract text from it and all of its descendants. 
		/// </param>
		/// <returns>
		/// All text contained in the supplied node and all of its descendants combined. 
		/// </returns>
		static std::string NodeText(const Node* node);

		/// <summary>
		/// Get the text of only the children of the supplied node.
		/// </summary>
		/// <param name="node">
		/// The node from which to extract the text contents of its children.
		/// </param>
		/// <returns>
		/// All text contained in the direct children of the supplied node combined.
		/// </returns>
		static std::string NodeOwnText(const Node* node);

		/// <summary>
		/// Checks if the supplied node to search for already exists inside the supplied collection
		/// of nodes.
		/// </summary>
		/// <param name="nodeCollection">
		/// The collection of nodes to search within. 
		/// </param>
		/// <param name="search">
		/// The node to search for. 
		/// </param>
		/// <returns>
		/// True if the supplied search node was found in the supplied collection of nodes, false
		/// otherwise.
		/// </returns>
		static bool NodeExists(const std::vector< const Node* >& nodeCollection, const GumboNode* search);

		/// <summary>
		/// Removes duplicate nodes from the collection.
		/// </summary>
		/// <param name="primaryCollection">
		/// The collection which may contain duplicate nodes.
		/// </param>
		static void RemoveDuplicates(std::vector< const Node* >& primaryCollection);

		/// <summary>
		/// Takes the supplied primary collection and adds any nodes to it contained in the second
		/// collection which it does not already contain. This is for merging new match results into
		/// existing match results without creating duplicate matches.
		/// <para>&#160;</para>
		/// Null checks are not performed. It is the responsibility of the user to ensure that the
		/// supplied collections are populated with valid pointers.
		/// </summary>
		/// <param name="primaryCollection">
		/// The primary collection containing unique nodes discovered during matching. This
		/// collection will have any nodes it does not already contain, which are found in the
		/// second collection, added to it.
		/// </param>
		/// <param name="collection">
		/// A collection of nodes to add to the primary collection only if they do not already exist
		/// in the primary collection.
		/// </param>
		/// <returns>
		/// </returns>
		static void UnionNodes(std::vector< const Node* >& primaryCollection, const std::vector< const Node* >& collection);

		/// <summary>
		/// Removes any quote characters at the first and last positions of the supplied string.
		/// Since we handle raw values instead of the values after Gumbo Parser has done conversions
		/// on things such as character references, the original values can/will still have any
		/// enclosing quotes present. In order to ensure proper matching, they must be trimmed.
		/// </summary>
		/// <param name="str">
		/// The string to trim enclosing quotation characters from. 
		/// </param>
		static boost::string_ref TrimEnclosingQuotes(boost::string_ref str);

		/// <summary>
		/// Removes leading and trailing whitespace from the supplied string_ref, returns the
		/// trimmed result.
		/// </summary>
		/// <param name="str">
		/// The string that may or may not contain leading and trailing whitespace.
		/// </param>
		/// <returns>
		/// The supplied string with any leading or trailing whitespace removed.
		/// </returns>
		static boost::string_ref Trim(boost::string_ref str);

		/// <summary>
		/// Gets the tag name for the supplied node. This method also handles unknown tags. That is,
		/// if the tag name is an unknown/non-standard tag name, the method will attempt to extract
		/// the tag name from the raw input.
		/// </summary>
		/// <param name="node">
		/// The node for which to get the normalized tag name.
		/// </param>
		/// <returns>
		/// The tag name of the supplied node.
		/// </returns>
		static std::string GetNodeTagName(const GumboNode* node);

	private:

		/// <summary>
		/// <para>&#160;</para>
		/// </summary>
		/// <param name="node"></param>
		/// <param name="stringContainer"></param>
		static void WriteNodeText(const GumboNode* node, std::string& stringContainer);

		Util();
		~Util();

	};

} /* namespace gq */