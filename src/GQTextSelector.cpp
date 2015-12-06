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

#include "GQTextSelector.hpp"
#include "GQUtil.hpp"
#include <boost/algorithm/string.hpp>

namespace gq
{

	GQTextSelector::GQTextSelector(const SelectorOperator op, const boost::string_ref value) :
		m_operator(op), m_textToMatch(value.to_string()), m_textToMatchStrRef(m_textToMatch)
	{
		if (m_textToMatch.size() == 0)
		{
			throw std::runtime_error(u8"In GQAttributeSelector::GQAttributeSelector(SelectorOperator, const boost::string_ref) - Supplied text to match has zero length.");
		}

		if (m_operator == SelectorOperator::Matches || m_operator == SelectorOperator::MatchesOwn)
		{
			m_expression.reset(new std::regex(m_textToMatch.c_str(), std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::nosubs));

			if (m_expression == nullptr)
			{
				throw std::runtime_error(u8"In GQAttributeSelector::GQAttributeSelector(SelectorOperator, const boost::string_ref) - Failed to allocate new std::regex for regex based GQTextSelector.");
			}
		}

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_SELECTOR_COMPILIATION
			std::cout << "Built GQTextSelector with operator " << static_cast<size_t>(m_operator) << " with text to match " << m_textToMatch << u8"." << std::endl;
			#endif
		#endif
	}

	GQTextSelector::GQTextSelector(const SelectorOperator op, std::string value) :
		m_operator(op), m_textToMatch(std::move(value)), m_textToMatchStrRef(m_textToMatch)
	{
		if (m_textToMatch.size() == 0)
		{
			throw std::runtime_error(u8"In GQAttributeSelector::GQAttributeSelector(SelectorOperator, std::string) - Supplied text to match has zero length.");
		}

		if (m_operator == SelectorOperator::Matches || m_operator == SelectorOperator::MatchesOwn)
		{
			m_expression.reset(new std::regex(m_textToMatch.c_str(), std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::nosubs));

			if (m_expression == nullptr)
			{
				throw std::runtime_error(u8"In GQAttributeSelector::GQAttributeSelector(SelectorOperator, const boost::string_ref) - Failed to allocate new std::regex for regex based GQTextSelector.");
			}
		}

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_SELECTOR_COMPILIATION
			std::cout << "Built GQTextSelector with operator " << static_cast<size_t>(m_operator) << " with text to match " << m_textToMatch << u8"." << std::endl;
			#endif
		#endif
	}

	GQTextSelector::~GQTextSelector()
	{

	}

	const bool GQTextSelector::Match(const GumboNode* node) const
	{		
		if (node == nullptr)
		{
			return false;
		}

		// This might seem strange. But, the idea behind text selectors is to match nodes that
		// eventually contain the text being searched for, not the text itself. If we allow text
		// nodes to be match results for this type of search, then the user will be expecting a 
		// returned list of a HTML elements that contain the text, and end up getting somewhere
		// in those results pure text nodes as well, which isn't really useful at all from a 
		// selector point of view.
		if (node->type == GUMBO_NODE_TEXT)
		{
			return false;
		}

		switch (m_operator)
		{
			case SelectorOperator::Contains:
			{
				auto text = GQUtil::NodeText(node);
				boost::string_ref textStrRef(text);
				auto searchResult = boost::find_first(textStrRef, m_textToMatchStrRef);
				return searchResult.empty() == false;
			}
			break;

			case SelectorOperator::ContainsOwn:
			{
				auto text = GQUtil::NodeOwnText(node);
				boost::string_ref textStrRef(text);
				auto searchResult = boost::find_first(textStrRef, m_textToMatchStrRef);
				return searchResult.empty() == false;
			}
			break;

			case SelectorOperator::Matches:
			{
				auto text = GQUtil::NodeText(node);
				return std::regex_search(text, *(m_expression.get()));
			}
			break;

			case SelectorOperator::MatchesOwn:
			{
				auto text = GQUtil::NodeOwnText(node);
				return std::regex_search(text, *(m_expression.get()));
			}
			break;
		}

		return false;
	}

} /* namespace gq */
