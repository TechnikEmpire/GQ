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

#include "GQAttributeSelector.hpp"
#include <boost/algorithm/string.hpp>

namespace gumboquery
{

	GQAttributeSelector::GQAttributeSelector(boost::string_ref key, const bool keyIsPrefix) :
		m_operator(SelectorOperator::Exists), 
		m_keyIsPrefix(keyIsPrefix), 
		m_attributeName(key.to_string()), 
		m_attributeNameRef(m_attributeName)
	{		
		if (m_attributeName.size() == 0)
		{
			throw new std::runtime_error(u8"In GQAttributeSelector::GQAttributeSelector(SelectorOperator, boost::string_ref, const bool) - Supplied attribute identifier has zero length.");
		}		
	}

	GQAttributeSelector::GQAttributeSelector(SelectorOperator op, boost::string_ref key, boost::string_ref value, const bool keyIsPrefix) :
		m_operator(op),
		m_keyIsPrefix(keyIsPrefix),
		m_attributeName(key.to_string()), 
		m_attributeNameRef(m_attributeName),
		m_attributeValue(value.to_string()),		
		m_attributeValueRef(m_attributeValue)
	{
		if (m_attributeName.size() == 0)
		{
			throw new std::runtime_error(u8"In GQAttributeSelector::GQAttributeSelector(SelectorOperator, boost::string_ref, const bool) - Supplied attribute identifier has zero length.");
		}

		if (m_attributeValue.size() == 0)
		{
			throw new std::runtime_error(u8"In GQAttributeSelector::GQAttributeSelector(SelectorOperator, boost::string_ref, const bool) - Supplied attribute value has zero length.");
		}

		if (m_operator == SelectorOperator::ValueContainsElementInWhitespaceSeparatedList)
		{
			if (m_attributeValue.find_first_of(u8"\t\r\n ") != std::string::npos)
			{
				throw new std::runtime_error(u8"In GQAttributeSelector::GQAttributeSelector(SelectorOperator, boost::string_ref, const bool) - Constructued ValueContainsElementInWhitespaceSeparatedList attribute selector, but spaces exist in the search value. This is not allowed.");
			}
		}
	}

	GQAttributeSelector::~GQAttributeSelector()
	{

	}

	const bool GQAttributeSelector::Match(const GumboNode* node) const
	{
		if (node->type != GUMBO_NODE_ELEMENT && node->type != GUMBO_NODE_TEXT && node->type != GUMBO_NODE_DOCUMENT)
		{
			return false;
		}

		GumboVector attributes = node->v.element.attributes;

		for (size_t i = 0; i < attributes.length; i++)
		{
			GumboAttribute* attribute = static_cast<GumboAttribute*>(attributes.data[i]);
			
			// Note that we take the original attribute name and value. This is because Gumbo Parser will
			// modify these things, such as replacing character references with literal characters. We don't
			// want to have to convert escaped character references in supplied selectors.
			boost::string_ref attributeName(attribute->original_name.data, attribute->original_name.length);
			boost::string_ref attributeValue(attribute->original_value.data, attribute->original_value.length);
			
			if (attributeName.size() == 0 || m_attributeName.size() > attributeName.size())
			{
				// Not possible to match if we have nothing to match against or if
				// the extracted name/value is greater than our value, not even prefix matches.
				continue;
			}

			// Since we're dealing with raw values, must remove and preceeding and trailing enclosing quotation characters
			TrimEnclosingQuotes(attributeName);
			//

			bool keyMatches = true;
			if (!boost::iequals(m_attributeName, attributeName))
			{
				// keys don't match, but perhaps our key is a prefix
				keyMatches = false;

				if (m_keyIsPrefix && m_attributeName.size() < attributeName.size())
				{
					// If the key is a prefix, then we just need to match a substr of the same length
					boost::string_ref sub = attributeName.substr(0, m_attributeName.size());

					if (boost::iequals(m_attributeName, sub))
					{
						keyMatches = true;
					}
				}			
			}

			if (!keyMatches)
			{
				continue;
			}

			switch (m_operator)
			{
				case SelectorOperator::Exists:
				{
					// We've already matched the attribute key, so we're done.
					return true;
				}
				break;

				case SelectorOperator::ValueContains:
				{
					// Case-insensitive search courtesy of boost
					auto searchResult = boost::ifind_first(attributeValue, m_attributeValue);

					// Simply return whether or not we got any matches.
					return !searchResult.empty();
				}
				break;

				case SelectorOperator::ValueEquals:
				{
					// Values cannot possibly be equal if not present or not the same size.
					if (attributeValue.size() == 0 || attributeValue.size() != m_attributeValue.size())
					{
						return false;
					}

					TrimEnclosingQuotes(attributeValue);

					return boost::iequals(attributeValue, m_attributeValue);
				}
				break;

				case SelectorOperator::ValueHasPrefix:
				{
					// If our prefix is greater than the attribute value, we can just move on.
					if (m_attributeValue.size() >= attributeValue.size())
					{
						return false;
					}

					TrimEnclosingQuotes(attributeValue);

					// Test case-insensitive equality of same-length substring.
					boost::string_ref sub = attributeValue.substr(0, m_attributeValue.size());
					
					return boost::iequals(sub, m_attributeValue);
				}
				break;

				case SelectorOperator::ValueHasSuffix:
				{
					// If our suffix is greater than the attribute value, we can just move on.
					if (m_attributeValue.size() >= attributeValue.size())
					{
						return false;
					}

					TrimEnclosingQuotes(attributeValue);

					// Test case-insensitive equality of same-length substring taken from the end.
					boost::string_ref sub = attributeValue.substr((attributeValue.size() - m_attributeValue.size()) -1);

					return boost::iequals(sub, m_attributeValue);
				}
				break;

				case SelectorOperator::ValueContainsElementInWhitespaceSeparatedList:
				{
					// If there isn't anything that qualifies as whitespace in the CSS selector world,
					// then we can just immediately return false.
					auto anySpacePosition = attributeValue.find_first_of("\t\r\n ");

					if (anySpacePosition == boost::string_ref::npos)
					{
						return false;
					}

					// There are spaces, so do a case-insensitive substring search.
					auto result = boost::ifind_first(attributeValue, m_attributeValue);

					// If our value isn't found, we can just return false.
					if (result.empty())
					{
						return false;
					}
					
					// Value was found, so now we need to check if the preceeding and following characters
					// around our match qualify as whitespace in the CSS selector world. 
					auto distance = std::distance(attributeValue.begin(), result.begin());

					// If the distance is zero, this can still be valid so long as the second check
					// passes. Since constructing attribute selectors that use this operator with a value
					// that contains spaces is illegal, it's simply not possible for us to have already 
					// found a space and then have the distance == 0 and also the length plus offset of our 
					// value member to also be equal to the length of the entire value to check against.
					// Therefore, at least one of these disqualifying checks is guaranteed to run and not 
					// further checks are required.

					if (distance > 0)
					{						
						const char& before = attributeValue[distance - 1];

						if (before != ' ' && before != '\t' && before != '\r' && before != '\n')
						{
							return false;
						}
					}

					if (static_cast<size_t>(distance + 1) < attributeValue.size())
					{
						const char& after = attributeValue[distance + attributeValue.length()];
						if (after != ' ' && after != '\t' && after != '\r' && after != '\n')
						{
							return false;
						}
					}

					// If one of the previously disqualifying checks didn't return false, then this
					// is necessarily a proper match.
					return true;

				}
				break;

				case SelectorOperator::ValueIsHyphenSeparatedListStartingWith:
				{
					// If the attribute value to check is smaller than our value, then we can just return
					// false right away.
					if (attributeValue.size() <= m_attributeValue.size())
					{
						return false;
					}
					
					// If we can't find a hypen, then we simply return false right away.
					auto anyHyphen = attributeValue.find_first_of('-');

					if (anyHyphen == boost::string_ref::npos)
					{
						return false;
					}

					TrimEnclosingQuotes(attributeValue);

					// A hyphen was found, so all we have to do is make a case-insensitive match against
					// a substring of equal length to our member value.
					boost::string_ref sub = attributeValue.substr(0, m_attributeValue.size());

					return boost::iequals(sub, m_attributeName);
				}
				break;
			}
		}

		return false;
	}

	void GQAttributeSelector::TrimEnclosingQuotes(boost::string_ref& str) const
	{
		if (str.length() > 1)
		{
			switch (str[0])
			{
				case '\'':
				case '"':
				{
					str = str.substr(1);
				}
				break;

				default:
					break;
			}

			switch (str[str.length() - 1])
			{
				case '\'':
				case '"':
				{
					str = str.substr(0, str.length() - 2);
				}
				break;

				default:
					break;
			}
		}		
	}

} /* namespace gumboquery */
