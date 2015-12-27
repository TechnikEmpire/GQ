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

#include "BinarySelector.hpp"
#include "Node.hpp"

namespace gq
{

	BinarySelector::BinarySelector(SelectorOperator op, SharedSelector left, SharedSelector right) :
		m_operator(op), m_leftHandSide(std::move(left)), m_rightHandSide(std::move(right))
	{
		#ifndef NDEBUG
			assert(m_leftHandSide != nullptr && m_rightHandSide != nullptr && u8"In BinarySelector::BinarySelector(SelectorOperator, SharedSelector, SharedSelector) - Left or right hand selector is nullptr.");
		#else
			if (m_leftHandSide == nullptr || m_rightHandSide == nullptr)
			{
				throw std::runtime_error(u8"In BinarySelector::BinarySelector(SelectorOperator, SharedSelector, SharedSelector) - Left or right hand selector is nullptr.");
			}
		#endif

		auto leftHandTagType = m_leftHandSide->GetTagTypeToMatch();
		auto rightHandTagType = m_rightHandSide->GetTagTypeToMatch();

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_DEBUG_NFO
			std::cout 
				<< u8"Built BinarySelector with operator " 
				<< static_cast<size_t>(m_operator) 
				<< u8" with left hand tag "
				<< static_cast<size_t>(leftHandTagType)
				<< u8" and right hand tag "
				<< static_cast<size_t>(rightHandTagType)
				<< u8"."
				<< std::endl;
			#endif
		#endif				

		switch (m_operator)
		{
			case SelectorOperator::Sibling:
			case SelectorOperator::Adjacent:
			{
				// We match the left hand side first, so lets take on its traits alone.
				auto& lhst = m_leftHandSide->GetMatchTraits();

				for (auto& tp : lhst)
				{
					AddMatchTrait(tp.first, tp.second);
				}
			}
			break;

			case SelectorOperator::Descendant:
			case SelectorOperator::Child:
			{
				// We match the right hand side first, so lets take on its traits alone.
				auto& rhst = m_rightHandSide->GetMatchTraits();

				for (auto& tp : rhst)
				{
					AddMatchTrait(tp.first, tp.second);
				}
			}
			break;

			case SelectorOperator::Union:
			case SelectorOperator::Intersection:
			{
				// Both traits need to match the same object, so we'll take on both traits. In the
				// case of union, either can match, so again, take on both.
				auto& lhst = m_leftHandSide->GetMatchTraits();
				auto& rhst = m_rightHandSide->GetMatchTraits();
				
				for (auto& tp : lhst)
				{
					AddMatchTrait(tp.first, tp.second);
				}

				for (auto& tp : rhst)
				{
					AddMatchTrait(tp.first, tp.second);
				}
			}
			break;
		}
	}

	BinarySelector::~BinarySelector()
	{

	}

	const Selector::MatchResult BinarySelector::Match(const Node* node) const
	{

		switch (m_operator)
		{
			case SelectorOperator::Adjacent:
			{
				const Node* parent = node->GetParent();
				if (parent == nullptr)
				{					
					return false;
				}

				if (node->GetIndexWithinParent() == 0)
				{
					// Adjacent right hand side must immediately follow the left hand side element.
					return false;
				}

				// Can't possibly have an adjacent sibling match without two children.
				if (parent->GetNumChildren() < 2)
				{
					// If there is only 1 child, then we cannot possibly match adjacent nodes.
					return false;
				}			

				auto prevSibling = parent->GetChildAt(node->GetIndexWithinParent() - 1);

				auto rhsResult = m_rightHandSide->Match(node);

				if (rhsResult && m_leftHandSide->Match(prevSibling) == true)
				{
					// We return the right-most match.
					return rhsResult;
				}
			}
			break;

			case SelectorOperator::Child:
			{
				const Node* parent = node->GetParent();

				// Can't be a child without a parent. Boo hoo );
				if (parent == nullptr)
				{
					return false;
				}

				auto rhsResult = m_rightHandSide->Match(node);

				if (rhsResult && m_leftHandSide->Match(node->GetParent()) == true)
				{
					return rhsResult;
				}
			}
			break;

			case SelectorOperator::Descendant:
			{
				const Node* parent = node->GetParent();

				// Can't be a descendant of the void, unless you're Xel'naga.
				if (parent == nullptr)
				{
					return false;
				}

				auto rhsResult = m_rightHandSide->Match(node);

				if (!rhsResult)
				{
					return rhsResult;
				}

				for (parent; parent != nullptr; parent = parent->GetParent())
				{
					if (m_leftHandSide->Match(parent) == true)
					{
						return rhsResult;
					}
				}

				return false;
			}
			break;

			case SelectorOperator::Intersection:
			{			
				auto rhsResult = m_rightHandSide->Match(node);

				if (rhsResult && m_leftHandSide->Match(node) == true)
				{
					return rhsResult;
				}

				return false;
			}
			break;

			case SelectorOperator::Sibling:
			{
				const Node* parent = node->GetParent();
				if (parent == nullptr)
				{
					return false;
				}

				if (node->GetIndexWithinParent() == 0)
				{
					// Adjacent right hand side must immediately follow the left hand side element.
					return false;
				}

				// Can't possibly have an adjacent sibling match without two children.
				if (parent->GetNumChildren() < 2)
				{
					// If there is only 1 child, then we cannot possibly match adjacent nodes.
					return false;
				}

				auto rhsResult = m_rightHandSide->Match(node);

				if (rhsResult == false)
				{
					return false;
				}

				auto numChildren = parent->GetNumChildren();
				
				for (size_t i = 0; i < numChildren; ++i)
				{
					if (i == node->GetIndexWithinParent())
					{
						continue;
					}

					auto sibling = parent->GetChildAt(i);

					if (m_leftHandSide->Match(sibling))
					{
						return rhsResult;
					}
				}

				return false;
			}
			break;

			case SelectorOperator::Union:
			{
				auto rhsResult = m_rightHandSide->Match(node);
				if (rhsResult)
				{
					return rhsResult;
				}

				auto lshResult = m_leftHandSide->Match(node);

				if (lshResult)
				{
					return lshResult;
				}

				return false;				
			}
			break;
		}

		return false;
	}

} /* namespace gq */
