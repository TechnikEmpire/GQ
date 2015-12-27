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

#include "Selector.hpp"

namespace gq
{
	/// <summary>
	/// The BinarySelector is designed to match two independent selectors against nodes in various
	/// ways, depending on the specified operator. This selector makes types of selection such as
	/// descendant and sibling matching possible.
	/// </summary>
	class BinarySelector final : public Selector
	{

	public:

		enum class SelectorOperator
		{
			/// <summary>
			/// A union selector allows multiple selectors to optionally match a given object, where
			/// as long as one of the selectors match is found, then the union selector declares a
			/// match. If neither match, then the union declares no match. These are constructed by
			/// placing comma between selectors.
			/// </summary>
			Union,

			/// <summary>
			/// An intersection selector requires that two distinct selectors both match against the
			/// same object in order to make a complete match. If a both match, then the intersection
			/// selector declares a match. If one or more do not match, then the intersection selector
			/// declares no match. 
			/// </summary>
			Intersection,

			/// <summary>
			/// Requires the right hand side of the child selector to match successfully against a
			/// child of the node matched by the left hand side. Both must return as matching for
			/// the child selector to declare a match. If one or more do not match, then the child
			/// selector declares no match.
			/// </summary>
			Child,

			/// <summary>
			/// Requires that the right hand side of the descendant selector matches a descendant of
			/// the node matched against the left hand side of the descendant selector. If both
			/// sides match, the descendant selector will declare a match. If one or more do not
			/// match, then the descendant selector declares no match.
			/// </summary>
			Descendant,

			/// <summary>
			/// Requires that the right hand side of the selector match a sibling node that that
			/// immediately follows the node matched by the left hand side. If a match is found for
			/// the left hand side, then the right hand side must match the immediate following
			/// sibling, both sharing the same parent. If both sides match, then adjacent selector
			/// will declare a match. If one or more do not match, then the adjacent selector
			/// declares no match.
			/// </summary>
			Adjacent,

			/// <summary>
			/// Requires that the left hand side and the right hand side both match nodes that have
			/// the same parent. Unlike the adjacent selector, the two matched nodes can be adjacent
			/// to one another, or they may not be. So long as they both share the same parent, it
			/// is a match. If a match is found for both sides and the matched nodes share the same
			/// parent, then the sibling selector will declare a match. If one or more sides do not
			/// match, then the sibling selector declares no match.
			/// </summary>
			Sibling
		};

		/// <summary>
		/// Constructs a binary selector with a left hand and right hand side, the two selectors
		/// linked to one another by the supplied operator. For example, if the operator is
		/// Adjascent, then the right hand side must match a node that is an immediate sibling of
		/// the node matched by the left hand side.
		/// <para>&#160;</para>
		/// Neither selector should be nullptr. If either selector argument is nullptr, this
		/// constructor will throw.
		/// </summary>
		/// <param name="op">
		/// The operator for matching. This value will define how the supplied selectors combine to
		/// form a match.
		/// </param>
		/// <param name="left">
		/// The left hand side of the combined selector. 
		/// </param>
		/// <param name="right">
		/// The right hand side of the combined selector. 
		/// </param>
		BinarySelector(SelectorOperator op, SharedSelector left, SharedSelector right);

		/// <summary>
		/// Default destructor.
		/// </summary>
		virtual ~BinarySelector();

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
		virtual const MatchResult Match(const Node* node) const;

	private:

		/// <summary>
		/// The left hand side of the binary selector to match against a node. 
		/// </summary>
		SharedSelector m_leftHandSide;

		/// <summary>
		/// The right hand side of the binary selector to match against a node.
		/// </summary>
		SharedSelector m_rightHandSide;

		/// <summary>
		/// The operator for the binary selector. The value of the operator will change how a true
		/// match is determined.
		/// </summary>
		SelectorOperator m_operator;
	};

} /* namespace gq */


