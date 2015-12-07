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

#include "GQUnarySelectory.hpp"

namespace gq
{

	GQUnarySelectory::GQUnarySelectory(SelectorOperator op, SharedGQSelector selector) :
		m_operator(op), m_selector(std::move(selector))
	{
		if (m_selector == nullptr)
		{
			throw std::runtime_error(u8"In GQUnarySelectory::GQUnarySelectory(SelectorOperator, SharedGQSelector) - Supplied shared selector is nullptr.");
		}

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_SELECTOR_COMPILIATION
			std::cout << "Built GQUnarySelectory with operator " << static_cast<size_t>(m_operator) << u8"." << std::endl;
			#endif
		#endif
	}

	GQUnarySelectory::~GQUnarySelectory()
	{
	}

	const bool GQUnarySelectory::GQUnarySelectory::Match(const GumboNode* node) const
	{
		// Don't match against stuff like comments, CDATA blocks, etc
		if (node->type != GUMBO_NODE_ELEMENT)
		{
			return false;
		}

		// If it's not a not selector, and there's no children, we can't do a child or descendant match
		if (m_operator != SelectorOperator::Not && node->v.element.children.length == 0)
		{
			return false;
		}

		switch (m_operator)
		{
			case SelectorOperator::Not:
			{
				return m_selector->Match(node) == false;
			}
			break;

			case SelectorOperator::HasDescendant:
			{
				return HasDescendantMatch(node);
			}
			break;

			case SelectorOperator::HasChild:
			{
				for (size_t i = 0; i < node->v.element.children.length; i++)
				{
					GumboNode* child = static_cast<GumboNode*>(node->v.element.children.data[i]);

					// Don't match against stuff like comments, CDATA blocks, etc
					if (child->type != GUMBO_NODE_ELEMENT && child->type != GUMBO_NODE_TEXT && child->type != GUMBO_NODE_DOCUMENT)
					{
						continue;
					}

					if (m_selector->Match(child))
					{
						return true;
					}
				}

				return false;				
			}
			break;	
		}

		// Just to shut up the compiler.
		return false;
	}

	const bool GQUnarySelectory::HasDescendantMatch(const GumboNode* node) const
	{
		if (node->type != GUMBO_NODE_ELEMENT)
		{
			return false;
		}

		for (size_t i = 0; i < node->v.element.children.length; i++)
		{
			GumboNode* child = static_cast<GumboNode*>(node->v.element.children.data[i]);

			// Don't match against stuff like comments, CDATA blocks, etc
			if (child->type != GUMBO_NODE_ELEMENT && child->type != GUMBO_NODE_TEXT && child->type != GUMBO_NODE_DOCUMENT)
			{
				continue;
			}

			if (m_selector->Match(child) || HasDescendantMatch(child))
			{
				return true;
			}
		}

		return false;
	}

} /* namespace gq */
