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

#include "GQBinarySelector.hpp"

namespace gq
{

	GQBinarySelector::GQBinarySelector(SelectorOperator op, SharedGQSelector left, SharedGQSelector right) :
		m_operator(op), m_leftHandSide(std::move(left)), m_rightHandSide(std::move(right))
	{
		if (m_leftHandSide == nullptr || m_rightHandSide == nullptr)
		{
			std::string errorMessage(u8"In GQBinarySelector::GQBinarySelector(SelectorOperator, SharedGQSelector, SharedGQSelector) - Left or right hand selector is nullptr. Supplied operator was ");
			errorMessage.append(std::to_string(static_cast<size_t>(m_operator))).append(u8".");
			throw std::runtime_error(errorMessage);
		}

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_SELECTOR_COMPILIATION
			std::cout << "Built GQBinarySelector with operator " << static_cast<size_t>(m_operator) << std::endl;
			#endif
		#endif
	}

	GQBinarySelector::~GQBinarySelector()
	{

	}

	const bool GQBinarySelector::Match(const GumboNode* node) const
	{
		switch (m_operator)
		{
			case SelectorOperator::Adjacent:
			{
				if (node->type != GUMBO_NODE_ELEMENT) 
				{
					return false;
				}

				if (node->parent == nullptr)
				{					
					return false;
				}

				if (node->index_within_parent == 0)
				{
					// Adjacent right hand side must immediately follow the left hand side element.
					return false;
				}

				// Can't possibly have an adjacent sibling match without two children.
				if (node->parent->v.element.children.length < 2)
				{
					// If there is only 1 child, then we cannot possibly match adjacent nodes.
					return false;
				}

				if (!m_rightHandSide->Match(node))
				{
					return false;
				}				

				// We don't want to match both sides to the same element, so start at the right
				// hand side position minus one.
				long rightHandPrevPosition = static_cast<long>((node->index_within_parent - 1));

				for (long i = rightHandPrevPosition; i >= 0; --i)
				{
					GumboNode* prevSibling = static_cast<GumboNode*>(node->parent->v.element.children.data[i]);
					if (prevSibling->type == GUMBO_NODE_TEXT || prevSibling->type == GUMBO_NODE_COMMENT)
					{
						continue;
					}

					return m_leftHandSide->Match(prevSibling);
				}
			}
			break;

			case SelectorOperator::Child:
			{
				if (node->type != GUMBO_NODE_ELEMENT) 
				{
					return false;
				}

				// Can't be a child without a parent. Boo hoo );
				if (node->parent == nullptr)
				{
					return false;
				}

				return m_rightHandSide->Match(node) && m_leftHandSide->Match(node->parent);
			}
			break;

			case SelectorOperator::Descendant:
			{
				if (node->type != GUMBO_NODE_ELEMENT) 
				{
					return false;
				}

				// Can't be a descendant of the void, unless you're Xel'naga.
				if (node->parent == nullptr)
				{
					return false;
				}

				if (!m_rightHandSide->Match(node))
				{
					return false;
				}

				for (GumboNode* p = node->parent; p != nullptr; p = p->parent)
				{
					if (m_leftHandSide->Match(p))
					{
						return true;
					}
				}

				return false;
			}
			break;

			case SelectorOperator::Intersection:
			{
				if (node->type != GUMBO_NODE_ELEMENT) 
				{
					return false;
				}

				return m_leftHandSide->Match(node) && m_rightHandSide->Match(node);
			}
			break;

			case SelectorOperator::Sibling:
			{
				if (node->type != GUMBO_NODE_ELEMENT) 
				{
					return false;
				}

				if (node->parent == nullptr)
				{
					return false;
				}

				if (node->index_within_parent == 0)
				{
					// Sibling right hand side must preceed the left hand side element.
					return false;
				}

				// Can't possibly have a sibling match without two children.
				if (node->parent->v.element.children.length < 2)
				{
					return false;
				}

				if (!m_rightHandSide->Match(node))
				{
					return false;
				}

				// We don't want to match both sides to the same element, so start at the right
				// hand side position minus one.
				long rightHandPrevPosition = static_cast<long>((node->index_within_parent - 1));

				for (long i = rightHandPrevPosition; i >= 0; --i)
				{
					GumboNode* other = static_cast<GumboNode*>(node->parent->v.element.children.data[i]);

					if (m_leftHandSide->Match(other))
					{
						return true;
					}
				}

				return false;
			}
			break;

			case SelectorOperator::Union:
			{
				if (node->type != GUMBO_NODE_ELEMENT) 
				{
					return false;
				}

				return m_leftHandSide->Match(node) || m_rightHandSide->Match(node);
			}
			break;
		}

		return false;
	}

} /* namespace gq */
