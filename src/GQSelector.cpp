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

#include "GQSelector.hpp"

namespace gumboquery
{

	GQSelector::GQSelector(SelectorOperator op = SelectorOperator::Dummy)
	{
		InitDefaults();
		m_selectorOperator = op;
	}

	GQSelector::GQSelector(const bool ofType)
	{
		InitDefaults();
		m_selectorOperator = SelectorOperator::OnlyChild;
		m_ofType = ofType;
	}

	GQSelector::GQSelector(const int a, const int b, const bool last, const bool ofType)
	{
		InitDefaults();
		m_selectorOperator = SelectorOperator::NthChild;
		m_a = a;
		m_b = b;
		m_last = last;
		m_ofType = ofType;
	}

	GQSelector::GQSelector(GumboTag tag)
	{
		InitDefaults();
		m_selectorOperator = SelectorOperator::Tag;
		m_tag = tag;
	}

	GQSelector::~GQSelector()
	{
	}

	const bool GQSelector::Match(const GumboNode* node) const
	{
		switch (m_selectorOperator)
		{
			case SelectorOperator::Dummy:
			{
				return true;
			}
			break;

			case SelectorOperator::Empty:
			{
				if (node->type != GUMBO_NODE_ELEMENT)
				{
					return false;
				}

				GumboVector children = node->v.element.children;

				for (size_t i = 0; i < children.length; i++)
				{
					GumboNode* child = (GumboNode*)children.data[i];
					if (child->type == GUMBO_NODE_TEXT || child->type == GUMBO_NODE_ELEMENT)
					{
						return false;
					}
				}

				return true;
			}
			break;

			case SelectorOperator::OnlyChild:
			{
				if (node->type != GUMBO_NODE_ELEMENT)
				{
					return false;
				}

				if (node->parent == nullptr)
				{
					// Can't be a child without parents. :( Poor node. So sad.
					return false;
				}

				int count = 0;
				
				for (size_t i = 0; i < node->parent->v.element.children.length; i++)
				{
					GumboNode* child = static_cast<GumboNode*>(node->parent->v.element.children.data[i]);
					if (child->type != GUMBO_NODE_ELEMENT || (m_ofType && node->v.element.tag == child->v.element.tag))
					{
						continue;
					}

					count++;

					if (count > 1)
					{
						return false;
					}
				}

				return count == 1;
			}
			break;

			case SelectorOperator::NthChild:
			{
				if (node->type != GUMBO_NODE_ELEMENT)
				{
					return false;
				}

				if (node->parent == nullptr)
				{
					// Can't be a child without parents. :( Poor node. So sad.
					return false;
				}

				size_t count = 0;
				size_t index = 0;
				for (size_t i = 0; i < node->parent->v.element.children.length; i++)
				{
					GumboNode* child = static_cast<GumboNode*>(node->parent->v.element.children.data[i]);
					if (child->type != GUMBO_NODE_ELEMENT || (m_ofType && node->v.element.tag == child->v.element.tag))
					{
						continue;
					}

					count++;

					if (node == child)
					{
						index = count;

						if (!m_last)
						{
							break;
						}
					}
				}

				if (m_last)
				{
					index = count - index + 1;
				}

				index -= m_b;

				if (m_a == 0)
				{
					return index == 0;
				}

				return index % m_a == 0 && index / m_a > 0;
			}
			break;

			case SelectorOperator::Tag:
			{
				return node->type == GUMBO_NODE_ELEMENT && node->v.element.tag == m_tag;
			}
			break;

			default:
				return false;
		}
	}

	std::vector<GumboNode*> GQSelector::MatchAll(const GumboNode* node) const
	{

	}

	void GQSelector::Filter(std::vector<GumboNode*>& nodes) const
	{

	}

	void GQSelector::InitDefaults()
	{
		m_ofType = false;
		m_a = 0;
		m_b = 0;
		m_last = false;
		m_tag = GumboTag(0);
	}

	void GQSelector::MatchAllInto(GumboNode* node, std::vector<GumboNode*>& nodes)
	{

	}
	

} /* namespace gumboquery */

