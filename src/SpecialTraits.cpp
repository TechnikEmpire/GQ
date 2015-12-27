/*
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

#include "GQSpecialTraits.hpp"
#include <random>

namespace gq
{

	GQSpecialTraits::GQSpecialTraits()
	{
	}

	GQSpecialTraits::~GQSpecialTraits()
	{
	}

	const boost::string_ref GQSpecialTraits::GetTagKey()
	{
		return TagKey.Get();
	}

	const boost::string_ref GQSpecialTraits::GetPseudoKey()
	{
		return PseudoKey.Get();
	}

	const boost::string_ref GQSpecialTraits::GetAnyValue()
	{
		// For values, we're actually safe to return static non-random values, because they are
		// sheltered by the uniqueness of the key.
		return boost::string_ref(u8"*");
	}

	const boost::string_ref GQSpecialTraits::GetLastChildValue()
	{		
		return boost::string_ref(u8"last-child");
	}

	const boost::string_ref GQSpecialTraits::GetLastChildOfTypeValue()
	{
		return boost::string_ref(u8"last-of-type");
	}

	GQSpecialTraits::RandomKey::RandomKey()
	{
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_real_distribution<double> dist(0., static_cast<double>(Chars.size() - 1));

		m_str.resize(10);

		m_str[0] = Chars[static_cast<size_t>(dist(mt))];
		m_str[1] = Chars[static_cast<size_t>(dist(mt))];
		m_str[2] = Chars[static_cast<size_t>(dist(mt))];
		m_str[3] = Chars[static_cast<size_t>(dist(mt))];
		m_str[4] = Chars[static_cast<size_t>(dist(mt))];
		m_str[5] = Chars[static_cast<size_t>(dist(mt))];
		m_str[6] = Chars[static_cast<size_t>(dist(mt))];
		m_str[7] = Chars[static_cast<size_t>(dist(mt))];
		m_str[8] = Chars[static_cast<size_t>(dist(mt))];
		m_str[9] = Chars[static_cast<size_t>(dist(mt))];
	}

	const boost::string_ref GQSpecialTraits::RandomKey::Get() const
	{
		return boost::string_ref(m_str);
	}

	const std::string GQSpecialTraits::RandomKey::Chars{ u8"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" };

	const GQSpecialTraits::RandomKey GQSpecialTraits::TagKey{};

	const GQSpecialTraits::RandomKey GQSpecialTraits::PseudoKey{};

} /* namespace gq */