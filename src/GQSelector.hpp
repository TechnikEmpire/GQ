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

// For printing debug information about compiled selectors to the console.
#ifndef NDEBUG
#include <iostream>
#endif

namespace gq
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

		/// <summary>
		/// Constructs a GQSelector intended solely to match a specific element based on its tag
		/// type.
		/// </summary>
		/// <param name="tagTypeToMatch">
		/// The type of tag/element to match. 
		/// </param>
		GQSelector(GumboTag tagTypeToMatch);

		/// <summary>
		/// Default destructor.
		/// </summary>
		virtual ~GQSelector();

		/// <summary>
		/// Gets the GumboTag that this selector is to match against. If this selector is not built with the ::Tag matching operator, then the
		/// result will be empty. Default value is empty/nothing. 
		/// </summary>
		/// <returns>
		/// The GumboTag, if any, that this selector is to match against.
		/// </returns>
		const GumboTag GetTagTypeToMatch() const;		
		
		virtual const bool Match(const GumboNode* node) const;

		std::vector< std::shared_ptr<GQNode> > MatchAll(const GumboNode* node) const;

		void Filter(std::vector< std::shared_ptr<GQNode> >& nodes) const;

	private:
		
		SelectorOperator m_selectorOperator = SelectorOperator::Dummy;

		/// <summary>
		/// If this selector was constructed to do an nth-* match, this member will hold the value
		/// of the left hand side (left of the N) of the nth parameter. In the event that the parsed
		/// value of the nth parameter is something like "odd", "even", "-n+3", the values supplied
		/// in the constructor will already be accurately set accordingly by
		/// GQParser::ParseNth(...).
		/// </summary>
		int m_leftHandSideOfNth = 0;

		/// <summary>
		/// If this selector was constructed to do an nth-* match, this member will hold the value
		/// of the right hand side (right of the N) of the nth parameter. In the event that the
		/// parsed value of the nth parameter is something like "odd", "even", "-n+3", the values
		/// supplied in the constructor will already be accurately set accordingly by
		/// GQParser::ParseNth(...).
		/// </summary>
		int m_rightHandSideOfNth = 0;

		/// <summary>
		/// If this member is true, then in our matching code, it means we're doing some sort of 
		/// last match, where we're takes to match the last or nth last or last-of-type.
		/// </summary>
		bool m_matchLast;

		/// <summary>
		/// This member is difficult to name correctly, in a way that makes its purpose very clear.
		/// This member has absolutely nothing to do with matching a GumboTag. Rather, this bool is
		/// used for correctly counting sublings and children in nth-child/only-child type selectors
		/// where type is a factor. For example, nth-last-of-type and only-of-type would both
		/// require this member to be set to "true". The reason for this is what when we're
		/// iterating through siblings/children on such selectors, we need to ignore anything not of
		/// the same type as the node we're trying to match when counting. nth-last-of-type(2) is
		/// very easy to calculate when you only count siblings/children of the same type.
		/// 
		/// I know I've explained this to death, but I want to save the user the burden I had of
		/// resolving the true meaning/purpose of this member.
		/// </summary>
		bool m_matchType;
		
		/// <summary>
		/// If the selector is a tag selector, this member will store the type of GumboTag the
		/// selector must match against.
		/// </summary>
		GumboTag m_tagTypeToMatch;

		/// <summary>
		/// Init member defaults across multiple constructors.
		/// </summary>
		void InitDefaults();

		void MatchAllInto(const GumboNode* node, std::vector< std::shared_ptr<GQNode> >& nodes) const;

	};

	typedef std::shared_ptr<GQSelector> SharedGQSelector;

} /* namespace gq */
