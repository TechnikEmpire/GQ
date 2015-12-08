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

namespace gq
{
	GQAttributeSelector::GQAttributeSelector(boost::string_ref key, const bool keyIsPrefix) :
		m_operator(SelectorOperator::Exists),
		m_keyIsPrefix(keyIsPrefix),
		m_attributeNameString(key.to_string()),
		m_attributeNameRef(m_attributeNameString)
	{
		if (m_attributeNameRef.size() == 0)
		{
			throw std::runtime_error(u8"In GQAttributeSelector::GQAttributeSelector(SelectorOperator, boost::string_ref, const bool) - Supplied attribute identifier has zero length.");
		}

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_SELECTOR_COMPILIATION
				std::cout << "Built Exists GQAttributeSelector with key " << m_attributeName << std::endl;
			#endif
		#endif
	}

	GQAttributeSelector::GQAttributeSelector(SelectorOperator op, boost::string_ref key, boost::string_ref value, const bool keyIsPrefix) :
		m_operator(op),
		m_keyIsPrefix(keyIsPrefix),
		m_attributeNameString(key.to_string()),
		m_attributeNameRef(m_attributeNameString),
		m_attributeValueString(value.to_string()),
		m_attributeValueRef(m_attributeValueString)
	{
		if (m_attributeNameRef.size() == 0)
		{
			throw std::runtime_error(u8"In GQAttributeSelector::GQAttributeSelector(SelectorOperator, boost::string_ref, const bool) - Supplied attribute identifier has zero length.");
		}

		if (m_attributeValueRef.size() == 0)
		{
			throw std::runtime_error(u8"In GQAttributeSelector::GQAttributeSelector(SelectorOperator, boost::string_ref, const bool) - Supplied attribute value has zero length.");
		}

		if (m_operator == SelectorOperator::ValueContainsElementInWhitespaceSeparatedList)
		{
			if (m_attributeNameRef.find_first_of(u8"\t\r\n ") != std::string::npos)
			{
				throw std::runtime_error(u8"In GQAttributeSelector::GQAttributeSelector(SelectorOperator, boost::string_ref, const bool) - Constructed ValueContainsElementInWhitespaceSeparatedList attribute selector, but spaces exist in the search value. This is not allowed.");
			}
		}

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_SELECTOR_COMPILIATION
				std::cout << "Built GQAttributeSelector with operator " << static_cast<size_t>(m_operator) << " with key " << m_attributeName << " looking for value " << m_attributeValue << std::endl;
			#endif
		#endif
	}

	GQAttributeSelector::~GQAttributeSelector()
	{
	}

	const bool GQAttributeSelector::Match(const GumboNode* node) const
	{		

		if (node->type != GUMBO_NODE_ELEMENT)
		{
			return false;
		}

		const GumboVector* attributes = &node->v.element.attributes;

		if (attributes == nullptr || attributes->length == 0)
		{
			return false;
		}

		// Note that we take the original attribute name and value. This is because Gumbo Parser will
		// modify these things, such as replacing character references with literal characters. We don't
		// want to have to convert escaped character references in supplied selectors.
		boost::string_ref attributeName;
		boost::string_ref attributeValue;

		bool foundAttribute = false;		

		auto nsz = m_attributeNameRef.size();

		if (m_keyIsPrefix)
		{
			for (size_t i = 0; i < attributes->length; i++)
			{
				GumboAttribute* attribute = static_cast<GumboAttribute*>(attributes->data[i]);

				if (attribute->original_name.length < nsz)
				{
					continue;
				}

				attributeName = boost::string_ref(attribute->original_name.data, nsz);
				attributeValue = boost::string_ref(attribute->original_value.data, attribute->original_value.length);

				if (attributeName[0] == m_attributeNameRef[0] && attributeName[nsz - 1] == m_attributeNameRef[nsz - 1])
				{
					if (attributeName.compare(m_attributeNameRef) == 0)
					{
						foundAttribute = true;
						break;
					}
				}				
			}
		}
		else
		{	
			for (size_t ai = 0; ai < node->v.element.attributes.length; ++ai)
			{
				GumboAttribute* attr = static_cast<GumboAttribute*>(node->v.element.attributes.data[ai]);
				if (attr->original_name.length != nsz)
				{
					continue;
				}				

				attributeName = boost::string_ref(attr->original_name.data, attr->original_name.length);

				if (attributeName[0] == m_attributeNameRef[0] && attributeName[1] == m_attributeNameRef[1] && attributeName[nsz - 1] == m_attributeNameRef[nsz - 1])
				{
					if (attributeName.compare(m_attributeNameRef) == 0)
					{
						attributeValue = boost::string_ref(attr->original_value.data, attr->original_value.length);
						foundAttribute = true;
						break;
					}
					
				}				
			}
		}

		if (!foundAttribute)
		{
			return false;
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
				// Just do a search
				auto searchResult = attributeValue.find(m_attributeValueRef);

				// Simply return whether or not we got any matches.
				return searchResult != boost::string_ref::npos;
			}
			break;

			case SelectorOperator::ValueEquals:
			{
				// Values cannot possibly be equal if not present or not the same size.
				if (attributeValue.size() == 0 || attributeValue.size() < m_attributeValueRef.size())
				{
					return false;
				}

				TrimEnclosingQuotes(attributeValue);

				if (attributeValue[0] == m_attributeValueRef[0] && attributeValue[m_attributeValueRef.size() - 1] == m_attributeValueRef[m_attributeValueRef.size() - 1])
				{
					return attributeValue.compare(m_attributeValueRef) == 0;
				}

				return false;
			}
			break;

			case SelectorOperator::ValueHasPrefix:
			{
				TrimEnclosingQuotes(attributeValue);

				// If our prefix is greater than the attribute value, we can just move on.
				if (attributeValue.size() == 0 || m_attributeValueRef.size() >= attributeValue.size())
				{
					return false;
				}				

				// Test case-insensitive equality of same-length substring.
				boost::string_ref sub = attributeValue.substr(0, m_attributeValueRef.size());

				if (sub[0] == m_attributeValueRef[0] && sub[m_attributeValueRef.size() - 1] == m_attributeValueRef[m_attributeValueRef.size() - 1])
				{
					return sub.compare(m_attributeValueRef) == 0;
				}

				return false;
			}
			break;

			case SelectorOperator::ValueHasSuffix:
			{
				TrimEnclosingQuotes(attributeValue);

				// If our suffix is greater than the attribute value, we can just move on.
				if (attributeValue.size() == 0 || m_attributeValueRef.size() >= attributeValue.size())
				{
					return false;
				}

				// Test case-insensitive equality of same-length substring taken from the end.
				boost::string_ref sub = attributeValue.substr((attributeValue.size() - m_attributeValueRef.size()));

				if (sub[0] == m_attributeValueRef[0] && sub[m_attributeValueRef.size() - 1] == m_attributeValueRef[m_attributeValueRef.size() - 1])
				{
					return sub.compare(m_attributeValueRef) == 0;
				}

				return false;
			}
			break;

			case SelectorOperator::ValueContainsElementInWhitespaceSeparatedList:
			{
				TrimEnclosingQuotes(attributeValue);

				// If the attribute value to check is smaller than our value, then we can just return
				// false right away.
				if (attributeValue.size() == 0 || attributeValue.size() < m_attributeValueRef.size())
				{
					return false;
				}

				if (attributeValue.size() == m_attributeValueRef.size())
				{
					// If the two values match exactly, this is considered a match with this selector
					// type. If they do not match, the only other possible type of match this operator
					// can make is the match the selector value PLUS whitespace, in which case this isn't
					// possible (being the two strings equal length), so letting boost::iequals return
					// false or true is the right answer either way.

					if (attributeValue[0] == m_attributeValueRef[0] && attributeValue[m_attributeValueRef.size() - 1] == m_attributeValueRef[m_attributeValueRef.size() - 1])
					{
						return attributeValue.compare(m_attributeValueRef) == 0;
					}

					return false;
				}

				// If there isn't anything that qualifies as whitespace in the CSS selector world,
				// then we can just immediately return false.
				auto anySpacePosition = attributeValue.find(' ');

				if (anySpacePosition == boost::string_ref::npos)
				{
					return false;
				}				
				
				auto firstSpace = attributeValue.find(' ');

				while (firstSpace != boost::string_ref::npos && attributeValue.size() > 0)
				{					
					if (firstSpace > 0 && firstSpace == m_attributeValueRef.size())
					{
						auto sub = attributeValue.substr(0, firstSpace);
						if (sub[0] == m_attributeValueRef[0] && sub[m_attributeValueRef.size()-1] == m_attributeValueRef[m_attributeValueRef.size()-1])
						{
							return sub.compare(m_attributeValueRef) == 0;
						}
					}

					attributeValue = attributeValue.substr(firstSpace + 1);
					firstSpace = attributeValue.find(' ');
				}

				return false;
			}
			break;

			case SelectorOperator::ValueIsHyphenSeparatedListStartingWith:
			{
				TrimEnclosingQuotes(attributeValue);

				// If the attribute value to check is smaller than our value, then we can just return
				// false right away.
				if (attributeValue.size() == 0 || attributeValue.size() < m_attributeValueRef.size())
				{
					return false;
				}

				if (attributeValue.size() == m_attributeValueRef.size())
				{
					// If the two values match exactly, this is considered a match with this selector
					// type. If they do not match, the only other possible type of match this operator
					// can make is the match the selector value PLUS a dash, in which case this isn't
					// possible (being the two strings equal length), so letting boost::iequals return
					// false or true is the right answer either way.

					if (attributeValue[0] == m_attributeValueRef[0] && attributeValue[m_attributeValueRef.size() - 1] == m_attributeValueRef[m_attributeValueRef.size() - 1])
					{
						return attributeValue.compare(m_attributeValueRef) == 0;
					}

					return false;
				}

				// If we didn't find an exact match, then the only hope of a match now is finding the selector
				// value at the start of the attribute value, immediately followed by a hyphen. Therefore, if
				// we can't find a hypen, then we simply return false right away.
				auto anyHyphen = attributeValue.find('-');

				if (anyHyphen == boost::string_ref::npos)
				{
					return false;
				}

				// A hyphen was found, so all we have to do is make a case-insensitive match against
				// a substring of equal length to our member value.
				boost::string_ref sub = attributeValue.substr(0, m_attributeValueRef.size() + 1);

				if (sub[sub.length() - 1] != '-')
				{
					// If the last character in the substring isn't a dash, it can't possibly be a match anyway.
					return false;
				}

				sub = attributeValue.substr(0, m_attributeValueRef.size());

				if (sub[0] == m_attributeValueRef[0] && sub[m_attributeValueRef.size() - 1] == m_attributeValueRef[m_attributeValueRef.size() - 1])
				{
					return sub.compare(m_attributeValueRef) == 0;
				}

				return false;
			}
			break;
		}

		return false;
	}

	void GQAttributeSelector::TrimEnclosingQuotes(boost::string_ref& str) const
	{
		if (str.length() >= 3)
		{
			switch (str[0])
			{
				case '\'':
				case '"':
				{
					if (str[str.length() - 1] == str[0])
					{
						str = str.substr(1, str.length() - 2);
					}
				}
				break;

				default:
					break;
			}			
		}
	}
} /* namespace gq */