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
#include <boost/utility/string_ref.hpp>
#include <regex>

namespace gq
{

	/// <summary>
	/// The GQTextSelector is designed to match specifically against text elements. There are four
	/// types of operations supported by the GQTextSelector.
	/// <para>&#160;</para>
	/// Match against a node where itself or any of its descendants contains a specific string.
	/// Match against a node where itself or any of its direct children contain a specific string.
	/// Match against a node where itself or any of its descendants contain text that is positively
	/// matched against by a regular expression. Match against a node where itself or any of its
	/// direct children contain text that is positively matched against by a regular expression.
	/// <para>&#160;</para>
	/// Currently the regex implementation is rigid in some regards. Regular expressions such as
	/// "/some.*stuff/gi" are not supported, because this would involve parsing the expression from
	/// the options and converting options to implementation specific flags, maybe. I say maybe
	/// because I didn't even bother investigating really, as I have not read the w3 standard on the
	/// matter. For time constraints some things such as this are presently left on the to do list.
	/// <para>&#160;</para>
	/// The grammar is specified to be ECMAScript which is supposed to be implemented as defined in
	/// ECMA-262 section [28.13].
	/// </summary>
	class GQTextSelector final : public GQSelector
	{

	public:

		enum class SelectorOperator
		{
			/// <summary>
			/// The Contains selector will match any node that contains the text to search for
			/// anywhere in that node or any of its descendants.
			/// </summary>
			Contains,

			/// <summary>
			/// The ContainsOwn selector will match any node where one or more of its children
			/// contains the text to search for.
			/// </summary>
			ContainsOwn,

			/// <summary>
			/// The Matches selector will match any node that contains text which the supplied
			/// regular expression matches against, be it within itself or any of its descendants.
			/// </summary>
			Matches,

			/// <summary>
			/// The MatchesOwn selector will match any node that contains text which the supplied
			/// regular expression matches against be it within itself, or any direct child node
			/// which contains such text.
			/// </summary>
			MatchesOwn
		};

		/// <summary>
		/// Constructs a new GQTextSelector with the supplied operator and text to match. The
		/// supplied text must have a length greater than zero. If it does not, this constructor
		/// will throw. It is not logical and therefore not possible to construct a selector with no
		/// parameter to function on.
		/// </summary>
		/// <param name="op">
		/// The operator for matching. This value will define how the supplied attribute value is
		/// matched.
		/// </param>
		/// <param name="value">
		/// The text to be matched. Must have a length greater than zero.
		/// </param>
		GQTextSelector(const SelectorOperator op, const boost::string_ref value);

		/// <summary>
		/// Constructs a new GQTextSelector with the supplied operator and text to match. The
		/// supplied text must have a length greater than zero. If it does not, this constructor
		/// will throw. It is not logical and therefore not possible to construct a selector with no
		/// parameter to function on.
		/// </summary>
		/// <param name="op">
		/// The operator for matching. This value will define how the supplied attribute value is
		/// matched.
		/// </param>
		/// <param name="value">
		/// The text to be matched. Must have a length greater than zero.
		/// </param>
		GQTextSelector(const SelectorOperator op, std::string value);

		/// <summary>
		/// Default destructor.
		/// </summary>
		virtual ~GQTextSelector();

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
		/// Defines how the matching in this selector will work. Based on the option, the text to be
		/// matched will be matched in different ways. See comments in the operator class.
		/// </summary>
		SelectorOperator m_operator;

		/// <summary>
		/// The text to match.
		/// </summary>
		std::string m_textToMatch;

		/// <summary>
		/// A boost::string_ref wrapper for the text to be matched.
		/// </summary>
		boost::string_ref m_textToMatchStrRef;

		/// <summary>
		/// If this text selector is regex based selector, we'll need a regex object to work with.
		/// If it's not, then we certainly don't want to incur the overhead of default constructed
		/// regexes needlessly. As such, this object will hold a unique_ptr to a regex which may or
		/// may not be constructed depending on the desired composition of this object, extrapolated
		/// from the operator(s) supplied to the constructors.
		/// </summary>
		std::unique_ptr<std::regex> m_expression;

	};

} /* namespace gq */


