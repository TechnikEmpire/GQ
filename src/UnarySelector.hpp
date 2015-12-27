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

#include "GQSelector.hpp"

namespace gq
{

	/// <summary>
	/// The UnarySelector class makes selections possible such as negative matching (:not), child
	/// and descendant matching.
	/// </summary>
	class GQUnarySelector final : public GQSelector
	{

	public:

		enum class SelectorOperator
		{
			/// <summary>
			/// A not selector declares a match when it is compared against a node that does not
			/// match anything specified in the not selector. If there is a match in the base
			/// selector, then the not selector declares no match.
			/// </summary>
			Not,

			/// <summary>
			/// A descendant selector declares a match when any child found down the tree from the
			/// node to match against is positively matched by the base selector. If no such child
			/// is found, then the descendant match declares no match. To further clarify, any
			/// possibly match must be able to link upwards through its parent linked list back to
			/// the node that is being compared for matching.
			/// </summary>
			HasDescendant,

			/// <summary>
			/// A child selector declares a match when any one of the children belonging to the node
			/// being matched against matches the base selector. If no direct child matches the base
			/// selector, then the child selector declares no match.
			/// </summary>
			HasChild
		};

		/// <summary>
		/// Constructs a unary selector with the supplied operator and base selector that is to be
		/// used to perform the matching. The supplied selector must be a valid shared pointer. If
		/// the supplied selector is nullptr, this constructor will throw.
		/// </summary>
		/// <param name="op">
		/// The operator for matching. This value will define how the supplied attribute value is
		/// matched.
		/// </param>
		/// <param name="selector">
		/// The base selector to be used to perform actual condition/compositional matching. 
		/// </param>
		GQUnarySelector(SelectorOperator op, SharedGQSelector selector);

		/// <summary>
		/// Default destructor.
		/// </summary>
		virtual ~GQUnarySelector();

		/// <summary>
		/// Check if this selector is a match against the supplied node. 
		/// </summary>
		/// <param name="node">
		/// The node to attempt to match against. 
		/// </param>
		/// <returns>
		/// True if this selector was successfully matched against the supplied node, false
		/// otherwise.
		/// </returns>
		virtual const GQMatchResult Match(const GQNode* node) const;

	private:

		/// <summary>
		/// Defines how the matching in this selector will work. Based on the option, the attribute
		/// name and value to be matched will be matched in different ways. See comments in the
		/// operator class.
		/// </summary>
		SelectorOperator m_operator;

		/// <summary>
		/// The base selector to be used to perform actual condition/compositional matching. The
		/// true kind of selector is not known nor does it matter. Whatever it is, its shares the
		/// same ::Match(...) override.
		/// </summary>
		SharedGQSelector m_selector;

		/// <summary>
		/// Proper descendant matching is recursive, so the descendant matching is separeted into
		/// this recursive function.
		/// </summary>
		/// <param name="node">
		/// The node containing children to recursively match against. 
		/// </param>
		/// <returns>
		/// True if any of the supplied nodes descendants was a match, false otherwise. 
		/// </returns>
		const GQMatchResult HasDescendantMatch(const GQNode* node) const;

	};

} /* namespace gq */
