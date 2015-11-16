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

#include "GQParser.hpp"

namespace gumboquery
{

	GQParser::GQParser()
	{
	}


	GQParser::~GQParser()
	{
	}

	SharedGQSelector GQParser::CreateSelector(const std::string& selectorString)
	{
		boost::string_ref input = boost::string_ref(selectorString);

		return CreateSelector(input);
	}

	SharedGQSelector GQParser::CreateSelector(const boost::string_ref selectorString)
	{
		m_input = selectorString;

		return ParseSelectorGroup();
	}

	SharedGQSelector GQParser::ParseSelectorGroup()
	{
		// Parse the first selector object from the input supplied.
		SharedGQSelector ret = ParseSelector();

		// ParseSelector() will stop if it encounters a character in the selector 
		// string that indicates that the supplied input is a selector group. That 
		// is, if "," is encountered while ParseSelector() is consuming input, it 
		// will break and return the most recently constructed GQSelector. So, 
		// after we get an initial return, we'll continue to recursively build 
		// selectors and combine them until no more "," group indicators are found 
		// and/or the end of the input has been reached. 
		if (m_offset < m_input.length() && m_input[m_offset] == ',')
		{
			m_offset++;

			while (m_offset < m_input.size())
			{
				SharedGQSelector sel = ParseSelector();
			}
		}
		
		return ret;
	}

	SharedGQSelector GQParser::ParseSelector()
	{

	}

	SharedGQSelector GQParser::ParseSimpleSelectorSequence()
	{

	}

	SharedGQSelector GQParser::ParsePseudoclassSelector()
	{

	}

	SharedGQSelector GQParser::ParseAttributeSelector()
	{

	}

	SharedGQSelector GQParser::ParseClassSelector()
	{

	}

	SharedGQSelector GQParser::ParseIDSelector()
	{

	}

	SharedGQSelector GQParser::ParseTypeSelector()
	{

	}

	void GQParser::ParseNth(const int& aA, const int& aB)
	{

	}

	int GQParser::ParseInteger()
	{

	}

	bool GQParser::ConsumeClosingParenthesis()
	{

	}

	bool GQParser::ConsumeParenthesis()
	{

	}

	bool GQParser::SkipWhitespace()
	{

	}

	std::string GQParser::ParseString()
	{

	}

	std::string GQParser::ParseName()
	{

	}

	std::string GQParser::ParseIdentifier()
	{

	}

	std::string GQParser::ParseEscape()
	{

	}

	bool GQParser::NameChar(const char c)
	{

	}

	bool GQParser::NameStart(const char c)
	{

	}

	bool GQParser::NexDigit(const char c)
	{

	}

} /* namespace gumboquery */
