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

#pragma once

#include <memory>
#include <string>
#include <gumbo.h>
#include <vector>
#include <cassert>

namespace gumboquery
{

	class GQNode;

	class GQSelector
	{

	public:

		enum class SelectorOperator
		{
			Dummy,
			Empty,
			OnlyChild,
			NthChild,
			Tag
		};

		GQSelector(SelectorOperator op = SelectorOperator::Dummy);

		/// <summary>
		/// Construct a selector meant to match a single element. 
		/// </summary>
		/// <param name="matchType">
		/// Determine if the single element is bound to a specific type or not. 
		/// </param>
		GQSelector(const bool matchType);

		/// <summary>
		/// Construct a selector used to match nth-(child|last-child|of-type|last-of-type)
		/// selectors. The arguments, a and b, are the parsed result of the selector Nth parameter,
		/// a value specified like "nth-child(3n+3)". These values, combined with other parameters
		/// such as matchLast determine how to correctly select based on the supplied nth arguments.
		/// </summary>
		/// <param name="leftHandSideOfNth">
		/// The left-hand side of the nth argument. In the example nth-child(2n+3), this value would
		/// be 2 (two).
		/// </param>
		/// <param name="rightHandSideOfNth">
		/// The right-hand side of the nth argument. In the example nth-child(2n+3), this value would
		/// be 3 (three).
		/// </param>
		/// <param name="matchLast">
		/// Determine if the matching is from the last, rather than the first. This will modify how
		/// the supplied offsets calculate which element to select.
		/// </param>
		/// <param name="matchType">
		/// Determine if matching is bound to a specific type. 
		/// </param>
		GQSelector(const int leftHandSideOfNth, const int rightHandSideOfNth, const bool matchLast, const bool matchType);

		GQSelector(GumboTag tagTypeToMatch);

		void SetTagTypeToMatch(GumboTag tagType);

		const GumboTag GetTagTypeToMatch(GumboTag tagType) const;

		virtual ~GQSelector();

		virtual const bool Match(const GumboNode* node) const;

		std::vector< std::shared_ptr<GQNode> > MatchAll(const GumboNode* node) const;

		void Filter(std::vector< std::shared_ptr<GQNode> >& nodes) const;

	private:
		
		SelectorOperator m_selectorOperator = SelectorOperator::Dummy;

		int m_leftHandSideOfNth = 0;
		int m_rightHandSideOfNth = 0;
		bool m_matchLast;
		bool m_matchType;
		GumboTag m_tagTypeToMatch;

		void InitDefaults();

		void MatchAllInto(const GumboNode* node, std::vector< std::shared_ptr<GQNode> >& nodes) const;

	};

	typedef std::shared_ptr<GQSelector> SharedGQSelector;

} /* namespace gumboquery */
