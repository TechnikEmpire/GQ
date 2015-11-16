/*
* This is a heavily modified fork of gumbo-query by Hoping White aka LazyTiger.
* The original software can be found at: https://github.com/lazytiger/gumbo-query
*
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
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
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
#include <stdexcept>
#include <vector>
#include "GQNode.hpp"
#include "GQSelector.hpp"

namespace gumboquery
{
	class GQSelection
	{

	public:

		GQSelection(const GumboNode* node);

		GQSelection(std::vector<GumboNode*>&& nodes);

		GQSelection(const std::vector<GumboNode*>& nodes);

		~GQSelection();

		/// <summary>
		/// Run a selector against the document and return any and all nodes that were matched by
		/// the supplied selector string.
		/// </summary>
		/// <param name="selectorString">
		/// The selector string to query against the document with. 
		/// </param>
		/// <returns>
		/// A collection of nodes that were matched by the supplied selector. If no matches were
		/// found, the collection will be empty.
		/// </returns>
		GQSelection Find(const std::string& selectorString) const;

		/// <summary>
		/// Run a selector against the document and return any and all nodes that were matched by
		/// the supplied selector string.
		/// </summary>
		/// <param name="selector">
		/// The precompiled selector object to query against the document with. 
		/// </param>
		/// <returns>
		/// A collection of nodes that were matched by the supplied selector. If no matches were
		/// found, the collection will be empty.
		/// </returns>
		GQSelection Find(const GQSelector& selector) const;

		/// <summary>
		/// Check the total number of nodes held in this selection. This number represents how many
		/// nodes were either supplied to this selection at construction, or were gathered into this
		/// selection as a result of them matching against a selector query that generated this
		/// object.
		/// </summary>
		/// <returns>
		/// The total number of nodes contained in this selection.
		/// </returns>
		const size_t& GetNodeCount() const;

		/// <summary>
		/// Fetch a copy of the node at the supplied index. The index must respect the upper and
		/// lower boundaries indicated by the GetNodeCount() method. That is to say, the index must
		/// be valid and this object must contain at least one node or this method will throw.
		/// </summary>
		/// <param name="index">
		/// The index of the node to fetch.
		/// </param>
		/// <returns>
		/// A copy of the node found at the supplied index.
		/// </returns>
		GQNode GetNodeAt(const size_t& index) const;

	private:

		std::vector<GumboNode*> m_nodes;

	};
} /* namespace gumboquery */
