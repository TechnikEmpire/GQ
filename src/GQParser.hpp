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

#include "GQAttributeSelector.hpp"
#include "GQBinarySelector.hpp"
#include "GQTextSelector.hpp"
#include "GQUnarySelectory.hpp"
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/algorithm/string.hpp>
#include <stdexcept>
#include <locale>

namespace gq
{
	class GQParser
	{

	public:

		GQParser();
		~GQParser();

		/// <summary>
		/// Create a selector instance from the supplied raw selector string. There are several
		/// potential issues that can arise in parsing, such as errors discovered in the syntax of
		/// the supplied rule string. This method should be wrapped in a try/catch.
		/// </summary>
		/// <param name="selectorString">
		/// The raw selector string to parse and compile into a selector. 
		/// </param>
		/// <returns>
		/// The compiled selector object. 
		/// </returns>
		static SharedGQSelector CreateSelector(std::string selectorString);

	private:

		/// <summary>
		/// Hash implementation for string_ref keys in unordered_maps.
		/// </summary>
		struct StringRefHasher
		{
			size_t operator()(const boost::string_ref& strRef) const
			{				
				return boost::hash_range(strRef.begin(), strRef.end());
			}
		};

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

		SharedGQSelector ParseSelectorGroup(boost::string_ref& selectorStr) const;

		SharedGQSelector ParseSelector(boost::string_ref& selectorStr) const;

		SharedGQSelector ParseSimpleSelectorSequence(boost::string_ref& selectorStr) const;

		SharedGQSelector ParsePseudoclassSelector(boost::string_ref& selectorStr) const;

		SharedGQSelector ParseAttributeSelector(boost::string_ref& selectorStr) const;

		SharedGQSelector ParseClassSelector(boost::string_ref& selectorStr) const;

		SharedGQSelector ParseIDSelector(boost::string_ref& selectorStr) const;

		SharedGQSelector ParseTypeSelector(boost::string_ref& selectorStr) const;

		void ParseNth(boost::string_ref& selectorStr, int& lhs, int& rhs) const;

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
		/// 
		/// Note that this function absolutely expects that the first character is either a single
		/// or double quote, and an acceptabled closing quote of the same kind is present in the
		/// string. Supplying a string to this function that does not meet those requirements will
		/// cause this function to throw.
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
		/// 
		/// So, ParseIdentifier was modified to skip over escaped character sequences, accepting
		/// them as a valid part of an identifier. Also, other code was put in place to convert
		/// character references to literal values as well. At this stage, I had removed a
		/// significant portion of the original code functionality, bypassing it all together, and
		/// embedded a significant change in how various values are interpreted and handled. Being
		/// wary, I have left this function for temporary legacy purposes until I can confirm with
		/// complete confidence that my changes are valid and this function can be removed and
		/// replaced with ::ParseIdentifier(...). It's left here to remind me.
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
		/// 
		/// In the original source, this function did not accept escaped character sequences, so
		/// when escaped characters were found elsewhere during the parsing process, and an
		/// identifier of some sort was expected, ::ParseName(...) would be called instead of
		/// ::ParseIdentifier(...), because ::ParseName(...) espected escaped character sequences
		/// and would convert them to literal values. On the other hand, if ParseIdentifier was
		/// called instead, but then an escaped character was found, it would throw.
		/// 
		/// As mentioned elsewhere, Gumbo Parser directly embeds escaped characters wherever it
		/// finds them, rather than converting them. Therefore, the previously described behavior is
		/// not appropriate and if these corrections are accurate, ParseName is no longer required
		/// at all. ParseIdentifier will accept and skip over escaped character sequences, so long
		/// as they are escaped correctly (\\ + hex value + space). This way, identifiers with such
		/// sequences will match correctly against the output of Gumbo Parser.
		/// </summary>
		/// <param name="selectorStr">
		/// The string from which to extract a identifier name. The identifier is extracted from the
		/// front end of the supplied string, removing it and placing it in the returned object.
		/// </param>
		/// <returns>
		/// The extracted identifier. 
		/// </returns>
		boost::string_ref ParseIdentifier(boost::string_ref& selectorStr) const;

		/* This function was in the original gumbo_query source code, but I've removed
		*  it because it actually breaks things. It was ported over from cascadia, but
		*  cascadia worked against a different parser, and as such it had to modify 
		*  selectors to work with that particular parser. See notes in the GQParser.cpp
		*  source file for more information on why this method was removed. I commented
		*  it out with explanation rather than simply mysteriously vanishing it so that
		*  the understanding of why it's wrong wouldn't be lost.
		boost::string_ref ParseEscape(boost::string_ref& selectorStr) const;
		*/

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
