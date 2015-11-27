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

#include "GQSelector.hpp"

#include "GQNode.hpp"

namespace gumboquery
{

	GQSelector::GQSelector(SelectorOperator op)
	{
		InitDefaults();
		m_selectorOperator = op;
	}

	GQSelector::GQSelector(const bool matchType)
	{
		InitDefaults();
		m_selectorOperator = SelectorOperator::OnlyChild;
		m_matchType = matchType;
	}

	GQSelector::GQSelector(const int leftHandSideOfNth, const int rightHandSideOfNth, const bool matchLast, const bool matchType)
	{
		InitDefaults();
		m_selectorOperator = SelectorOperator::NthChild;
		m_leftHandSideOfNth = leftHandSideOfNth;
		m_rightHandSideOfNth = rightHandSideOfNth;
		m_matchLast = matchLast;
		m_matchType = matchType;
	}

	GQSelector::GQSelector(GumboTag tagTypeToMatch)
	{
		InitDefaults();
		m_selectorOperator = SelectorOperator::Tag;
		m_tagTypeToMatch = tagTypeToMatch;
	}

	GQSelector::~GQSelector()
	{
	}

	void GQSelector::SetTagTypeToMatch(GumboTag tagType)
	{
		if (tagType == GUMBO_TAG_UNKNOWN)
		{
			m_matchType = false;
		}
		else
		{
			m_matchType = true;
		}

		m_tagTypeToMatch = tagType;
	}

	const GumboTag GQSelector::GetTagTypeToMatch(GumboTag tagType) const
	{
		return m_tagTypeToMatch;
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
				if (node->type != GUMBO_NODE_ELEMENT && node->type != GUMBO_NODE_TEXT && node->type != GUMBO_NODE_DOCUMENT)
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
				if (node->type != GUMBO_NODE_ELEMENT && node->type != GUMBO_NODE_TEXT && node->type != GUMBO_NODE_DOCUMENT)
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
					if (child->type != GUMBO_NODE_ELEMENT || (m_matchType && node->v.element.tag == child->v.element.tag))
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
				if (node->type != GUMBO_NODE_ELEMENT && node->type != GUMBO_NODE_TEXT && node->type != GUMBO_NODE_DOCUMENT)
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
					if (child->type != GUMBO_NODE_ELEMENT || (m_matchType && node->v.element.tag == child->v.element.tag))
					{
						continue;
					}

					count++;

					if (node == child)
					{
						index = count;

						if (!m_matchLast)
						{
							break;
						}
					}
				}

				if (m_matchLast)
				{
					index = count - index + 1;
				}

				index -= m_rightHandSideOfNth;

				if (m_leftHandSideOfNth == 0)
				{
					return index == 0;
				}

				return ((index % m_leftHandSideOfNth) == 0) && (static_cast<int>(index / m_leftHandSideOfNth) > 0);
			}
			break;

			case SelectorOperator::Tag:
			{
				if (node->type != GUMBO_NODE_ELEMENT && node->type != GUMBO_NODE_TEXT && node->type != GUMBO_NODE_DOCUMENT)
				{
					return false;
				}

				return node->v.element.tag == m_tagTypeToMatch;
			}
			break;

			default:
				return false;
		}
	}

	std::vector< std::shared_ptr<GQNode> > GQSelector::MatchAll(const GumboNode* node) const
	{
		#ifndef NDEBUG
		assert(node != nullptr && u8"In GQSelector::MatchAll(const GumboNode*) - Nullptr node supplied for matching.");
		#else
		if (node == nullptr) { throw new std::runtime_error(u8"In GQSelector::MatchAll(const GumboNode*) - Nullptr node supplied for matching."); }
		#endif

		std::vector< std::shared_ptr<GQNode> > ret;

		MatchAllInto(node, ret);

		return ret;
	}

	void GQSelector::Filter(std::vector< std::shared_ptr<GQNode> >& nodes) const
	{

	}

	void GQSelector::InitDefaults()
	{
		m_matchType = false;
		m_leftHandSideOfNth = 0;
		m_rightHandSideOfNth = 0;
		m_matchLast = false;
		m_tagTypeToMatch = GUMBO_TAG_UNKNOWN;
	}

	void GQSelector::MatchAllInto(const GumboNode* node, std::vector< std::shared_ptr<GQNode> >& nodes) const
	{
		#ifndef NDEBUG
		assert(node != nullptr && u8"In GQSelector::MatchAllInto(const GumboNode*, std::vector<const GumboNode*>&) - Nullptr node supplied for matching.");
		#else
		if (node == nullptr) { throw new std::runtime_error(u8"In GQSelector::MatchAllInto(const GumboNode*, std::vector<const GumboNode*>&) - Nullptr node supplied for matching."); }
		#endif

		if (Match(node))
		{
			nodes.push_back(std::make_shared<GQNode>(node));
		}

		if (node->type != GUMBO_NODE_ELEMENT)
		{
			return;
		}

		for (size_t i = 0; i < node->v.element.children.length; i++)
		{
			GumboNode* child = static_cast<GumboNode*>(node->v.element.children.data[i]);
			MatchAllInto(child, nodes);
		}
	}
	

} /* namespace gumboquery */

