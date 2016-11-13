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

#include "UnarySelector.hpp"
#include "Node.hpp"

namespace gq
{

	UnarySelector::UnarySelector(SelectorOperator op, SharedSelector selector) :
		m_operator(op), m_selector(std::move(selector))
	{
		if (m_selector == nullptr)
		{
			throw std::runtime_error(u8"In UnarySelector::UnarySelector(SelectorOperator, SharedSelector) - Supplied shared selector is nullptr.");
		}

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_DEBUG_NFO
				std::cout << "Built UnarySelector with operator " << static_cast<size_t>(m_operator) << u8"." << std::endl;
			#endif
		#endif

		// Take on the traits of the selector.
		auto& t = m_selector->GetMatchTraits();
		for (auto& tp : t)
		{
			AddMatchTrait(tp.first, tp.second);
		}
	}

	UnarySelector::~UnarySelector()
	{
	}

	const Selector::MatchResult UnarySelector::UnarySelector::Match(const Node* node) const
	{

		// If it's not a not selector, and there's no children, we can't do a child or descendant match
		if (m_operator != SelectorOperator::Not && node->GetNumChildren() == 0)
		{
			return nullptr;
		}

		switch (m_operator)
		{
			case SelectorOperator::Not:
			{
				auto result = m_selector->Match(node);

				if (result == false)
				{
					return MatchResult(node);
				}
				else
				{
					return result;
				}
			}
			break;

			case SelectorOperator::HasDescendant:
			{
				if (HasDescendantMatch(node))
				{
					// In the event of a :has/:haschild selector, you're interested in selecting a
					// particular parent that has a particular child, so we'll return the parent
					// here on a match.
					return MatchResult(node);
				}

				return nullptr;
			}
			break;

			case SelectorOperator::HasChild:
			{
				auto nNumChildren = node->GetNumChildren();
			
				for (size_t i = 0; i < nNumChildren; i++)
				{
					auto child = node->GetChildAt(i);

					auto childMatch = m_selector->Match(child);

					if (childMatch)
					{
						// In the event of a :has/:haschild selector, you're interested in selecting
						// a particular parent that has a particular child, so we'll return the
						// parent here on a match.
						return MatchResult(node);
					}
				}

				return nullptr;				
			}
			break;	
		}

		// Just to shut up the compiler.
		return nullptr;
	}

	const Selector::MatchResult UnarySelector::HasDescendantMatch(const Node* node) const
	{
		for (size_t i = 0; i < node->GetNumChildren(); i++)
		{
			auto child = node->GetChildAt(i);

			auto childMatch = m_selector->Match(child);
			if (childMatch)
			{
				return MatchResult(node);
			}

			childMatch = HasDescendantMatch(child);

			if (childMatch)
			{
				return MatchResult(node);
			}
		}

		return nullptr;
	}

} /* namespace gq */
