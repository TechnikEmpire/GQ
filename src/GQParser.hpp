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
#include <unordered_map>
#include "GQStrRefHash.hpp"
#include <stdexcept>
#include <locale>

namespace gq
{

	/// <summary>
	/// The GQParser class takes a selector string as input and will "compile" it into a finalized
	/// CSS selector for use against GQDocument and GQNode objects.
	/// </summary>
	class GQParser
	{

	public:

		/// <summary>
		/// Construcst a new GQParser.
		/// </summary>
		GQParser();

		/// <summary>
		/// Default destructor.
		/// </summary>
		~GQParser();

		/// <summary>
		/// Create a selector instance from the supplied raw selector string. There are several
		/// potential issues that can arise in parsing, such as errors discovered in the syntax of
		/// the supplied rule string. This method should be wrapped in a try/catch.
		/// </summary>
		/// <param name="selectorString">
		/// The raw selector string to parse and compile into a selector. 
		/// </param>
		/// <param name="retainOriginalString">
		/// If true, the original string will be copied into the returned selector. This is not
		/// necessary, and is only recommended for debugging selectors. Default is false.
		/// </param>
		/// <returns>
		/// The compiled selector object. 
		/// </returns>
		SharedGQSelector CreateSelector(std::string selectorString, const bool retainOriginalString = false) const;

	private:		

		/// <summary>
		/// Enumeration of the pseudo selectors supported by the parser.
		/// </summary>
		enum class PseudoOp
		{
			Not,
			Has,
			HasChild,
			Contains,
			ContainsOwn,
			Matches,
			MatchesOwn,
			NthChild,
			NthLastChild,
			NthOfType,
			NthLastOfType,
			FirstChild,
			LastChild,
			FirstOfType,
			LastOfType,
			OnlyChild,
			OnlyOfType,
			Empty
		};

		/// <summary>
		/// For quickly validating a parsed pseduo selector and in return getting an enum we can
		/// switch on rather than having a mess of if/elseif/else.
		/// </summary>
		static const std::unordered_map<boost::string_ref, PseudoOp, StringRefHasher> PseudoOps;

		/// <summary>
		/// For supplying to isalpha, etc.
		/// </summary>
		std::locale m_localeEnUS;

		/// <summary>
		/// Attempts to extract one selector, or two or more combined selectors, and returns the
		/// final result. Will either return the single selector, or combine multiple selectors
		/// separated by a "," into a chain of GQBinarySelector objects using the Union operator,
		/// which are also represented by a single GQSelector which is the topmost element in the
		/// chain.
		/// <para>&#160;</para>
		/// As with all other parsing members, this function will consume characters from the front
		/// of the supplied string. If no error is thrown, the consumption should leave the supplied
		/// string in a valid state that the parser can continue from. If an error is thrown, the
		/// supplied input will most certainly be mangled as the comsumption most likely would have
		/// stopped at an undefined state.
		/// </summary>
		/// <param name="selectorStr">
		/// The string containing a single selector or multiple grouped selectors. 
		/// </param>
		/// <returns>
		/// The compiled selector. 
		/// </returns>
		SharedGQSelector ParseSelectorGroup(boost::string_ref& selectorStr) const;

		/// <summary>
		/// Attempts top build one or more selector from the supplied string. Will handle all matter
		/// of combinators except for grouping (","). On encountering grouping or EOF, a single
		/// selector object is returned. However, this may be an individual selector, a chain of
		/// selectors where the returned GQSelector is the topmost element in the chain. This
		/// function delegates parsing individual selectors to methods such as
		/// ::ParseSimpleSelectorSequence(...) and then handles the combining, if any, internally.
		/// <para>&#160;</para>
		/// As with all other parsing members, this function will consume characters from the front
		/// of the supplied string. If no error is thrown, the consumption should leave the supplied
		/// string in a valid state that the parser can continue from. If an error is thrown, the
		/// supplied input will most certainly be mangled as the comsumption most likely would have
		/// stopped at an undefined state.
		/// </summary>
		/// <param name="selectorStr">
		/// The string containing a single selector or multiple combined selectors. 
		/// </param>
		/// <returns>
		/// The compiled selector. 
		/// </returns>
		SharedGQSelector ParseSelector(boost::string_ref& selectorStr) const;

		/// <summary>
		/// Attempts to parse a single selector from the supplied string. This object exists on
		/// encountering a combinator of some sort. This function will delegate construction of
		/// selectors to other members such as ::ParsePseudoclassSelector(...),
		/// ::ParseAttributeSelector(...) depending on the type of selector that this function has
		/// determined needs to be parsed based on the input string.
		/// <para>&#160;</para>
		/// As with all other parsing members, this function will consume characters from the front
		/// of the supplied string. If no error is thrown, the consumption should leave the supplied
		/// string in a valid state that the parser can continue from. If an error is thrown, the
		/// supplied input will most certainly be mangled as the comsumption most likely would have
		/// stopped at an undefined state.
		/// </summary>
		/// <param name="selectorStr">
		/// The string containing a single selector or multiple combined selectors. 
		/// </param>
		/// <returns>
		/// The compiled selector. 
		/// </returns>
		SharedGQSelector ParseSimpleSelectorSequence(boost::string_ref& selectorStr) const;

		/// <summary>
		/// Parses a single pseudo class selector from the supplied string. Returns a single,
		/// non-chained selector which may or may not be combined into a selector chain object. That
		/// is left up to invoking methods such as ::ParseSelector(...).
		/// <para>&#160;</para>
		/// As with all other parsing members, this function will consume characters from the front
		/// of the supplied string. If no error is thrown, the consumption should leave the supplied
		/// string in a valid state that the parser can continue from. If an error is thrown, the
		/// supplied input will most certainly be mangled as the comsumption most likely would have
		/// stopped at an undefined state.
		/// </summary>
		/// <param name="selectorStr">
		/// The string containing a single selector or multiple combined selectors. 
		/// </param>
		/// <returns>
		/// The compiled selector. 
		/// </returns>
		SharedGQSelector ParsePseudoclassSelector(boost::string_ref& selectorStr) const;

		/// <summary>
		/// Parses a single attribute selector from the supplied string. Returns a single,
		/// non-chained selector which may or may not be combined into a selector chain object. That
		/// is left up to invoking methods such as ::ParseSelector(...).
		/// <para>&#160;</para>
		/// As with all other parsing members, this function will consume characters from the front
		/// of the supplied string. If no error is thrown, the consumption should leave the supplied
		/// string in a valid state that the parser can continue from. If an error is thrown, the
		/// supplied input will most certainly be mangled as the comsumption most likely would have
		/// stopped at an undefined state.
		/// </summary>
		/// <param name="selectorStr">
		/// The string containing a single selector or multiple combined selectors. 
		/// </param>
		/// <returns>
		/// The compiled selector. 
		/// </returns>
		SharedGQSelector ParseAttributeSelector(boost::string_ref& selectorStr) const;

		/// <summary>
		/// Parses a single class selector from the supplied string. Returns a single,
		/// non-chained selector which may or may not be combined into a selector chain object. That
		/// is left up to invoking methods such as ::ParseSelector(...).
		/// <para>&#160;</para>
		/// As with all other parsing members, this function will consume characters from the front
		/// of the supplied string. If no error is thrown, the consumption should leave the supplied
		/// string in a valid state that the parser can continue from. If an error is thrown, the
		/// supplied input will most certainly be mangled as the comsumption most likely would have
		/// stopped at an undefined state.
		/// </summary>
		/// <param name="selectorStr">
		/// The string containing a single selector or multiple combined selectors. 
		/// </param>
		/// <returns>
		/// The compiled selector. 
		/// </returns>
		SharedGQSelector ParseClassSelector(boost::string_ref& selectorStr) const;

		/// <summary>
		/// Parses a ID selector from the supplied string. Returns a single, non-chained selector
		/// which may or may not be combined into a selector chain object. That is left up to
		/// invoking methods such as ::ParseSelector(...).
		/// <para>&#160;</para>
		/// As with all other parsing members, this function will consume characters from the front
		/// of the supplied string. If no error is thrown, the consumption should leave the supplied
		/// string in a valid state that the parser can continue from. If an error is thrown, the
		/// supplied input will most certainly be mangled as the comsumption most likely would have
		/// stopped at an undefined state.
		/// </summary>
		/// <param name="selectorStr">
		/// The string containing a single selector or multiple combined selectors. 
		/// </param>
		/// <returns>
		/// The compiled selector. 
		/// </returns>
		SharedGQSelector ParseIDSelector(boost::string_ref& selectorStr) const;

		/// <summary>
		/// Parses a GQSelector that matches against a specific HTML element by its tag type from
		/// the supplied string. Returns a single, non-chained selector which may or may not be
		/// combined into a selector chain object. That is left up to invoking methods such as
		/// ::ParseSelector(...).
		/// <para>&#160;</para>
		/// As with all other parsing members, this function will consume characters from the front
		/// of the supplied string. If no error is thrown, the consumption should leave the supplied
		/// string in a valid state that the parser can continue from. If an error is thrown, the
		/// supplied input will most certainly be mangled as the comsumption most likely would have
		/// stopped at an undefined state.
		/// </summary>
		/// <param name="selectorStr">
		/// The string containing a single selector or multiple combined selectors. 
		/// </param>
		/// <returns>
		/// The compiled selector. 
		/// </returns>
		SharedGQSelector ParseTypeSelector(boost::string_ref& selectorStr) const;

		/// <summary>
		/// Attempts to parse the left hand and right hand side of the supplied nth parameter
		/// string. Handles nth notation such as -n4, 3n+2, odd/even etc.
		/// <para>&#160;</para>
		/// As with all other parsing members, this function will consume characters from the front
		/// of the supplied string. If no error is thrown, the consumption should leave the supplied
		/// string in a valid state that the parser can continue from. If an error is thrown, the
		/// supplied input will most certainly be mangled as the comsumption most likely would have
		/// stopped at an undefined state.
		/// </summary>
		/// <param name="selectorStr">
		/// The string containing nth parameter. It is expected that the opening bracket has been
		/// consumed before being supplied here.
		/// </param>
		/// <param name="lhs">
		/// The left hand side reference to set on successful parsing. 
		/// </param>
		/// <param name="rhs">
		/// The right hand side reference to set on successful parsing. 
		/// </param>
		void ParseNth(boost::string_ref& selectorStr, int& lhs, int& rhs) const;

		/// <summary>
		/// Attempts to extract a number from the front of the supplied selector string, converting
		/// it to an integer.
		/// <para>&#160;</para>
		/// As with all other parsing members, this function will consume characters from the front
		/// of the supplied string. If no error is thrown, the consumption should leave the supplied
		/// string in a valid state that the parser can continue from. If an error is thrown, the
		/// supplied input will most certainly be mangled as the comsumption most likely would have
		/// stopped at an undefined state.
		/// </summary>
		/// <param name="selectorStr">
		/// The string containing the integer. It is expected that the supplied string begins with a
		/// digit character or whitespace immediately followed by a widget character.
		/// </param>
		/// <returns>
		/// The extracted integer value. 
		/// </returns>
		const int ParseInteger(boost::string_ref& selectorStr) const;

		/// <summary>
		/// Trims any whitspace that may be between the start of the supplied selector string and a
		/// closing parenthesis. This function expects that absolutely nothing but whitespace is
		/// (optionally) contained in the front of the string leading, leading up to a closing
		/// parenthesis. If these conditions are not met, this function will throw because either
		/// the supplied selector string is fundamentally broken, or the user has made an error in
		/// logic by calling this method when it ought not to be.
		/// </summary>
		/// <param name="selectorStr">
		/// The string beginning with either whitespace and a closing parenthesis, or just a closing
		/// parenthesis.
		/// </param>
		void ConsumeClosingParenthesis(boost::string_ref& selectorStr) const;

		/// <summary>
		/// Consumes the opening parenthesis at the beginning of the supplied string, and any
		/// whitespace found immediately following the opening parenthesis present at position 0. If
		/// an opening parenthesis is not found at position zero in the supplied string, this
		/// function will throw, because either the supplied selector string is fundamentally
		/// broken, or the user has mad an error in logic by calling this method when it ought not
		/// to be.
		/// <para>&#160;</para>
		/// As with all other parsing members, this function will consume characters from the front
		/// of the supplied string. If no error is thrown, the consumption should leave the supplied
		/// string in a valid state that the parser can continue from. If an error is thrown, the
		/// supplied input will most certainly be mangled as the comsumption most likely would have
		/// stopped at an undefined state.
		/// </summary>
		/// <param name="selectorStr">
		/// The string beginning with an opening parenthesis, optionally followed by whitespace
		/// characters to be consumed as well.
		/// </param>
		void ConsumeOpeningParenthesis(boost::string_ref& selectorStr) const;

		/// <summary>
		/// Removes any whitespace characters found in the start of the supplied string.
		/// </summary>
		/// <param name="str">
		/// The string that may contain leading whitespace.
		/// </param>
		/// <returns>
		/// True if leading whitespace characters were removed, false otherwise.
		/// </returns>
		const bool TrimLeadingWhitespace(boost::string_ref& str) const;

		/// <summary>
		/// Extracts all contents contained between an unescaped opening quote character, and the
		/// first unescaped closing quote character of the same kind. That is to say, if the
		/// supplied string begins with a single quote, then all of the contents between that first
		/// character and the first discovered single quote not preceeded by a forward slash ("\")
		/// are extracted and returned.
		/// <para>&#160;</para>
		/// Note that this function absolutely expects that the first character is either a single
		/// or double quote, and an acceptabled closing quote of the same kind is present in the
		/// string. Supplying a string to this function that does not meet those requirements will
		/// cause this function to throw.
		/// <para>&#160;</para>
		/// As with all other parsing members, this function will consume characters from the front
		/// of the supplied string. If no error is thrown, the consumption should leave the supplied
		/// string in a valid state that the parser can continue from. If an error is thrown, the
		/// supplied input will most certainly be mangled as the comsumption most likely would have
		/// stopped at an undefined state.
		/// </summary>
		/// <param name="selectorStr">
		/// A valid string starting with an unescsaped quote character, and a matching unescaped
		/// closing character present.
		/// </param>
		/// <returns>
		/// The substring containing the contents between the opening quote character and first
		/// matched closing quote character.
		/// </returns>
		boost::string_ref ParseString(boost::string_ref& selectorStr) const;

		/// <summary>
		/// Delegates directly to ParseIdentifier. This function will most likely be entirely
		/// removed in later versions. The ParseName function in the original source code differed
		/// from the ParseIdentifier in the fact that it accepted escaped character sequences and
		/// would automatically convert them to literal values. I found after doing this conversion
		/// that this behavior is not acceptable for using with Gumbo Parser, as Gumbo Parser
		/// ignores escaped character sequences, embedding them directly, while also converting
		/// character references.
		/// <para>&#160;</para>
		/// So, ParseIdentifier was modified to accept escaped characters, and skip over named or
		/// numbered character sequences, accepting them as a valid part of an identifier. At this
		/// stage, I had removed a significant portion of the original code functionality, bypassing
		/// it all together, and embedded a significant change in how various values are interpreted
		/// and handled. Being wary, I have left this function for temporary legacy purposes until I
		/// can confirm with complete confidence that my changes are valid and this function can be
		/// removed and replaced with ::ParseIdentifier(...). It's left here to remind me.
		/// <para>&#160;</para>
		/// As with all other parsing members, this function will consume characters from the front
		/// of the supplied string. If no error is thrown, the consumption should leave the supplied
		/// string in a valid state that the parser can continue from. If an error is thrown, the
		/// supplied input will most certainly be mangled as the comsumption most likely would have
		/// stopped at an undefined state.
		/// </summary>
		/// <param name="selectorStr">
		/// The string from which to extract a valid name. The name is extracted from the front end
		/// of the supplied string, removing it and placing it in the returned object.
		/// </param>
		/// <returns>
		/// The extracted name. 
		/// </returns>
		boost::string_ref ParseName(boost::string_ref& selectorStr) const;

		/// <summary>
		/// Extracts a valid identifier from the front of the supplied string, removing it from the
		/// supplied string and replacing it in the return object.
		/// <para>&#160;</para>
		/// Accepts named character and numbered character references as valid portions of an
		/// indetifier. Also accepts escaped characters like "\E9 " as valid portions of an
		/// identifier. The format for escaped characters must be followed, however. Must be a
		/// slash, followed by ONLY valid hex digits, then a single space. Anything else will throw.
		/// <para>&#160;</para>
		/// As with all other parsing members, this function will consume characters from the front
		/// of the supplied string. If no error is thrown, the consumption should leave the supplied
		/// string in a valid state that the parser can continue from. If an error is thrown, the
		/// supplied input will most certainly be mangled as the comsumption most likely would have
		/// stopped at an undefined state.
		/// </summary>
		/// <param name="selectorStr">
		/// The string from which to extract a identifier name. The identifier is extracted from the
		/// front end of the supplied string, removing it and placing it in the returned object.
		/// </param>
		/// <returns>
		/// The extracted identifier. 
		/// </returns>
		boost::string_ref ParseIdentifier(boost::string_ref& selectorStr) const;

		/// <summary>
		/// Checks whether the supplied character is a valid named entity character. Internally uses
		/// std::isalpha with en_US-UTF8 locale.
		/// </summary>
		/// <param name="c">
		/// The character to validate. 
		/// </param>
		/// <returns>
		/// True of the supplied character is valid for a named entity, false otherwise. 
		/// </returns>
		const bool IsNameChar(const char& c) const;

		/// <summary>
		/// Checks if the supplied character is a valid starting character for a named entity.
		/// Internally uses std::isalpha with en_US-UTF8 locale.
		/// </summary>
		/// <param name="c">
		/// The character to validate. 
		/// </param>
		/// <returns>
		/// True of the supplied character is a valid starting character for a named entity, false
		/// otherwise.
		/// </returns>
		const bool IsNameStart(const char& c) const;

		/// <summary>
		/// Checks if the supplied character is a valid CSS selector combinator. 
		/// </summary>
		/// <param name="c">
		/// The character to validate. 
		/// </param>
		/// <returns>
		/// True if the supplied character is a valid CSS selector combinator, false otherwise. 
		/// </returns>
		const bool IsCombinator(const char& c) const;

		/// <summary>
		/// Check if the supplied character is a "special" character in CSS selectors. Not
		/// necessarily a complete list. This function is meant to determine if certain escaped
		/// characters found in identifiers are valid escaped characters, aka special characters
		/// escaped that are commonly found in selector identifiers. For example, ":" can appear in
		/// divs.
		/// </summary>
		/// <param name="c">
		/// The character to check. 
		/// </param>
		/// <returns>
		/// True if the supplied character is "special", false otherwise. 
		/// </returns>
		const bool IsSpecial(const char& c) const;

		/// <summary>
		/// Checks if the supplied character is a valid hexidecimal digit. 
		/// </summary>
		/// <param name="c">
		/// The character to validate. 
		/// </param>
		/// <returns>
		/// True if the supplied character is a valid hexidecimal digit, false otherwise. 
		/// </returns>
		const bool IsHexDigit(const char& c) const;		

	};
} /* namespace gq */
