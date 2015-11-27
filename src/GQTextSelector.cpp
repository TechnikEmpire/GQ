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

namespace gumboquery
{

	GQTextSelector::GQTextSelector(const SelectorOperator op, const boost::string_ref value) :
		m_operator(op), m_textToMatch(value.to_string()), m_textToMatchStrRef(value)
	{
		if (m_textToMatch.size() == 0)
		{
			throw new std::runtime_error(u8"In GQAttributeSelector::GQAttributeSelector(SelectorOperator, const boost::string_ref) - Supplied text to match has zero length.");
		}
	}

	GQTextSelector::GQTextSelector(const SelectorOperator op, std::string value) :
		m_operator(op), m_textToMatch(std::move(value)), m_textToMatchStrRef(m_textToMatch)
	{
		if (m_textToMatch.size() == 0)
		{
			throw new std::runtime_error(u8"In GQAttributeSelector::GQAttributeSelector(SelectorOperator, std::string) - Supplied text to match has zero length.");
		}
	}

	GQTextSelector::~GQTextSelector()
	{

	}

	const bool GQTextSelector::Match(const GumboNode* node) const
	{		
		if (node == false)
		{
			return false;
		}

		switch (m_operator)
		{
			case SelectorOperator::Contains:
			{
				auto text = GQUtil::NodeText(node);
				boost::string_ref textStrRef(text);
				auto searchResult = boost::ifind_first(textStrRef, m_textToMatchStrRef);
				return !searchResult.empty();
			}
			break;

			case SelectorOperator::ContainsOwn:
			{
				auto text = GQUtil::NodeOwnText(node);
				boost::string_ref textStrRef(text);
				auto searchResult = boost::ifind_first(textStrRef, m_textToMatchStrRef);
				return !searchResult.empty();
			}
			break;
		}

		return false;
	}

} /* namespace gumboquery */
