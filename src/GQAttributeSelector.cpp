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

#include <cstring>
#include "GQAttributeSelector.hpp"
#include <boost/algorithm/string.hpp>
#include "GQNode.hpp"
#include "GQUtil.hpp"
#include "GQSpecialTraits.hpp"

namespace gq
{
	GQAttributeSelector::GQAttributeSelector(boost::string_ref key) :
		m_operator(SelectorOperator::Exists),
		m_attributeNameString(key.to_string()),
		m_attributeNameRef(m_attributeNameString)
	{
		if (m_attributeNameRef.size() == 0)
		{
			throw std::runtime_error(u8"In GQAttributeSelector::GQAttributeSelector(SelectorOperator, boost::string_ref, const bool) - Supplied attribute identifier has zero length.");
		}

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_DEBUG_NFO
				std::cout << "Built Exists GQAttributeSelector with key " << m_attributeNameRef << std::endl;
			#endif
		#endif

		// Add attribute key as a match trait for EXISTS, specifying any ("*") as the value.
		AddMatchTrait(m_attributeNameRef, GQSpecialTraits::GetAnyValue());
	}

	GQAttributeSelector::GQAttributeSelector(SelectorOperator op, boost::string_ref key, boost::string_ref value) :
		m_operator(op),
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
			#ifdef GQ_VERBOSE_DEBUG_NFO
				std::cout << "Built GQAttributeSelector with operator " << static_cast<size_t>(m_operator) << " with key " << m_attributeNameRef << " looking for value " << m_attributeValueRef << std::endl;
			#endif
		#endif

		switch (m_operator)
		{
			case SelectorOperator::ValueEquals:
			case SelectorOperator::ValueContainsElementInWhitespaceSeparatedList:
			{
				AddMatchTrait(m_attributeNameRef, m_attributeValueRef);
			}
			break;

			case SelectorOperator::Exists:
			case SelectorOperator::ValueContains:
			case SelectorOperator::ValueHasPrefix:
			case SelectorOperator::ValueHasSuffix:
			case SelectorOperator::ValueIsHyphenSeparatedListStartingWith:
			{
				AddMatchTrait(m_attributeNameRef, GQSpecialTraits::GetAnyValue());
			}
			break;
		}
	}

	GQAttributeSelector::~GQAttributeSelector()
	{
	}

	const GQSelector::GQMatchResult GQAttributeSelector::Match(const GQNode* node) const
	{		
		switch (m_operator)
		{
			case SelectorOperator::Exists:
			{						
				if (node->HasAttribute(m_attributeNameRef))
				{
					return GQMatchResult(node);
				}
			}
			break;

			case SelectorOperator::ValueContains:
			{
				auto attributeValue = node->GetAttributeValue(m_attributeNameRef);

				if (attributeValue.size() == 0)
				{
					return false;
				}

				// Just do a search
				auto searchResult = attributeValue.find(m_attributeValueRef);

				// Simply return whether or not we got any matches.
				if (searchResult != boost::string_ref::npos)
				{
					return GQMatchResult(node);
				}
			}
			break;

			case SelectorOperator::ValueEquals:
			{
				auto attributeValue = node->GetAttributeValue(m_attributeNameRef);

				auto oneSize = attributeValue.size();
				auto twoSize = m_attributeValueRef.size();

				if (oneSize == 0 || oneSize != twoSize)
				{
					return false;
				}

				if (oneSize >= 4)
				{
					if ((attributeValue[0] == m_attributeValueRef[0]) &&
						(attributeValue[1] == m_attributeValueRef[1]) &&
						(attributeValue[oneSize - 1] == m_attributeValueRef[oneSize - 1]) &&
						(attributeValue[oneSize - 2] == m_attributeValueRef[oneSize - 2]))
					{
						if (std::memcmp(attributeValue.begin(), m_attributeValueRef.begin(), oneSize) == 0)
						{
							return GQMatchResult(node);
						}
					}
				}
				else
				{
					if (std::memcmp(attributeValue.begin(), m_attributeValueRef.begin(), oneSize) == 0)
					{
						return GQMatchResult(node);
					}
				}

				return false;
			}
			break;

			case SelectorOperator::ValueHasPrefix:
			{
				auto attributeValue = node->GetAttributeValue(m_attributeNameRef);
						
				auto subSize = m_attributeValueRef.size();

				if (attributeValue.size() == 0 || attributeValue.size() <= subSize)
				{					
					return false;
				}				

				auto sub = attributeValue.substr(0, subSize);

				subSize = sub.size();

				if (subSize == m_attributeValueRef.size())
				{
					if (subSize >= 4)
					{
						if ((sub[0] == m_attributeValueRef[0]) &&
							(sub[1] == m_attributeValueRef[1]) &&
							(sub[subSize - 1] == m_attributeValueRef[subSize - 1]) &&
							(sub[subSize - 2] == m_attributeValueRef[subSize - 2]))
						{
							if (std::memcmp(sub.begin(), m_attributeValueRef.begin(), subSize) == 0)
							{
								return GQMatchResult(node);
							}
						}
					}
					else
					{
						if (std::memcmp(sub.begin(), m_attributeValueRef.begin(), subSize) == 0)
						{
							return GQMatchResult(node);
						}
					}
				}

				return false;
			}
			break;

			case SelectorOperator::ValueHasSuffix:
			{
				auto attributeValue = node->GetAttributeValue(m_attributeNameRef);

				auto subSize = m_attributeValueRef.size();

				// If our suffix is greater than the attribute value, we can just move on.
				if (attributeValue.size() == 0 || subSize >= attributeValue.size())
				{
					return false;
				}

				// Test equality of same-length substring taken from the end.
				boost::string_ref sub = attributeValue.substr((attributeValue.size() - subSize));

				subSize = sub.size();

				if (subSize == m_attributeValueRef.size())
				{
					if (subSize >= 4)
					{
						if ((sub[0] == m_attributeValueRef[0]) &&
							(sub[1] == m_attributeValueRef[1]) &&
							(sub[subSize - 1] == m_attributeValueRef[subSize - 1]) &&
							(sub[subSize - 2] == m_attributeValueRef[subSize - 2]))
						{
							if (std::memcmp(sub.begin(), m_attributeValueRef.begin(), subSize) == 0)
							{
								return GQMatchResult(node);
							}
						}
					}
					else
					{
						if (std::memcmp(sub.begin(), m_attributeValueRef.begin(), subSize) == 0)
						{
							return GQMatchResult(node);
						}
					}
				}

				return false;
			}
			break;

			case SelectorOperator::ValueContainsElementInWhitespaceSeparatedList:
			{
				auto attributeValue = node->GetAttributeValue(m_attributeNameRef);

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

					auto oneSize = attributeValue.size();

					if (oneSize >= 4)
					{
						if ((attributeValue[0] == m_attributeValueRef[0]) && 
							(attributeValue[1] == m_attributeValueRef[1]) &&
							(attributeValue[oneSize - 1] == m_attributeValueRef[oneSize - 1]) &&
							(attributeValue[oneSize - 2] == m_attributeValueRef[oneSize - 2]))
						{
							if (std::memcmp(attributeValue.begin(), m_attributeValueRef.begin(), oneSize) == 0)
							{
								return GQMatchResult(node);
							}
						}
					}
					else
					{
						if (std::memcmp(attributeValue.begin(), m_attributeValueRef.begin(), oneSize) == 0)
						{
							return GQMatchResult(node);
						}
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

						auto subSize = sub.size();

						if (subSize == m_attributeValueRef.size())
						{
							if (subSize >= 4)
							{
								if ((sub[0] == m_attributeValueRef[0]) &&
									(sub[1] == m_attributeValueRef[1]) &&
									(sub[subSize - 1] == m_attributeValueRef[subSize - 1]) &&
									(sub[subSize - 2] == m_attributeValueRef[subSize - 2]))
								{
									if (std::memcmp(sub.begin(), m_attributeValueRef.begin(), subSize) == 0)
									{
										return GQMatchResult(node);
									}
								}
							}
							else
							{
								if (std::memcmp(sub.begin(), m_attributeValueRef.begin(), subSize) == 0)
								{
									return GQMatchResult(node);
								}
							}
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
				auto attributeValue = node->GetAttributeValue(m_attributeNameRef);

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

					auto oneSize = attributeValue.size();

					if (oneSize >= 4)
					{
						if ((attributeValue[0] == m_attributeValueRef[0]) &&
							(attributeValue[1] == m_attributeValueRef[1]) &&
							(attributeValue[oneSize - 1] == m_attributeValueRef[oneSize - 1]) &&
							(attributeValue[oneSize - 2] == m_attributeValueRef[oneSize - 2]))
						{
							if (std::memcmp(attributeValue.begin(), m_attributeValueRef.begin(), oneSize) == 0)
							{
								return GQMatchResult(node);
							}
						}
					}
					else
					{
						if (std::memcmp(attributeValue.begin(), m_attributeValueRef.begin(), oneSize) == 0)
						{
							return GQMatchResult(node);
						}
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

				auto subSize = sub.size();

				if (subSize == m_attributeValueRef.size())
				{
					if (subSize >= 4)
					{
						if ((sub[0] == m_attributeValueRef[0]) &&
							(sub[1] == m_attributeValueRef[1]) &&
							(sub[subSize - 1] == m_attributeValueRef[subSize - 1]) &&
							(sub[subSize - 2] == m_attributeValueRef[subSize - 2]))
						{
							if (std::memcmp(sub.begin(), m_attributeValueRef.begin(), subSize) == 0)
							{
								return GQMatchResult(node);
							}
						}
					}
					else
					{
						if (std::memcmp(sub.begin(), m_attributeValueRef.begin(), subSize) == 0)
						{
							return GQMatchResult(node);
						}
					}
				}		

				return false;
			}
			break;
		}

		return false;
	}

} /* namespace gq */