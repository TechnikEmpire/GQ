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

#include "GQSelection.hpp"
#include "GQParser.hpp"
#include "GQUtil.hpp"

namespace gq
{

	GQSelection::GQSelection(SharedGQNode node)
	{
		if (node == nullptr)
		{
			throw std::runtime_error(u8"In GQSelection::GQSelection(SharedGQNode) - The SharedGQNode is nullptr.");
		}

		m_nodes.push_back(std::move(node));
	}

	GQSelection::GQSelection(std::vector<SharedGQNode>& nodes) :
		m_nodes(std::move(nodes))
	{

	}

	GQSelection::~GQSelection()
	{

	}

	GQSelection GQSelection::Find(const std::string& selectorString) const
	{
		GQParser parser;

		SharedGQSelector selector = parser.CreateSelector(selectorString);
	
		return Find(selector);
	}

	GQSelection GQSelection::Find(const SharedGQSelector& selector) const
	{
		std::vector<SharedGQNode> ret;

		for (std::vector<SharedGQNode>::const_iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
		{
			const GumboNode* node = (*it)->m_node;

			if (node == nullptr || node->type != GUMBO_NODE_ELEMENT)
			{
				continue;
			}
			
			std::vector<SharedGQNode> matched;
			selector->MatchAll(node, matched);
			GQUtil::UnionNodes(ret, matched);
		}

		return GQSelection(ret);
	}

	const size_t GQSelection::GetNodeCount() const
	{
		return m_nodes.size();
	}

	SharedGQNode GQSelection::GetNodeAt(const size_t index) const
	{
		if (m_nodes.size() == 0 || index >= m_nodes.size())
		{
			throw std::runtime_error(u8"In GQSelection::GetNodeAt(const size_t) - The supplied index is out of bounds.");
		}

		return m_nodes[index];
	}

} /* namespace gq */
