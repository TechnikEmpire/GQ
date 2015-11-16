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

#include "GQSelection.hpp"

namespace gumboquery
{

	GQSelection::GQSelection(const GumboNode* node)
	{
	}

	GQSelection::GQSelection(std::vector<GumboNode*>&& nodes) : 
		m_nodes(std::move(nodes))
	{

	}

	GQSelection::GQSelection(const std::vector<GumboNode*>& nodes) :
		m_nodes(nodes)
	{

	}

	GQSelection::~GQSelection()
	{
	}

	GQSelection GQSelection::Find(const std::string& selectorString) const
	{

	}

	GQSelection GQSelection::Find(const GQSelector& selector) const
	{

	}

	const size_t& GQSelection::GetNodeCount() const
	{
		m_nodes.size();
	}

	GQNode GQSelection::GetNodeAt(const size_t& index) const
	{
		if (m_nodes.size() == 0)
		{
			throw new std::runtime_error(u8"In GQSelection::GetNodeAt(const size_t&) - ::GetNodeCount() == 0. Cannot access any element by any valid index when the collection is empty.");
		}
		else if (index >= m_nodes.size())
		{
			throw new std::runtime_error(u8"In GQSelection::GetNodeAt(const size_t&) - index >= ::GetNodeCount(). The supplied index is out of bounds.");
		}

		return GQNode(m_nodes[index]);
	}

} /* namespace gumboquery */
