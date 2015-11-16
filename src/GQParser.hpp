/*
* This is a heavily modified fork of gumbo-query by Hoping White aka LazyTiger.
* The original software can be found at: https://github.com/lazytiger/gumbo-query
*
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
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
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

namespace gumboquery
{
	class GQParser
	{

	public:

		GQParser();
		~GQParser();

		SharedGQSelector CreateSelector(const std::string& selectorString);

		SharedGQSelector CreateSelector(const boost::string_ref selectorString);

	private:

		SharedGQSelector ParseSelectorGroup();

		SharedGQSelector ParseSelector();

		SharedGQSelector ParseSimpleSelectorSequence();

		SharedGQSelector ParsePseudoclassSelector();

		SharedGQSelector ParseAttributeSelector();

		SharedGQSelector ParseClassSelector();

		SharedGQSelector ParseIDSelector();

		SharedGQSelector ParseTypeSelector();

		void ParseNth(const int& aA, const int& aB);

		int ParseInteger();

		bool ConsumeClosingParenthesis();

		bool ConsumeParenthesis();

		bool SkipWhitespace();

		std::string ParseString();

		std::string ParseName();

		std::string ParseIdentifier();

		std::string ParseEscape();

		bool NameChar(const char c);

		bool NameStart(const char c);

		bool NexDigit(const char c);

		boost::string_ref m_input;

		size_t m_offset = 0;

	};
} /* namespace gumboquery */
