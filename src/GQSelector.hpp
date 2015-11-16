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

#include <memory>
#include <string>
#include <boost/utility/string_ref.hpp>
#include <gumbo.h>
#include <vector>

namespace gumboquery
{
	class GQSelector
	{

	public:

		enum class SelectorOperator
		{
			Dummy,
			Empty,
			OnlyChild,
			NthChild,
			Tag
		};

		GQSelector(SelectorOperator op = SelectorOperator::Dummy);

		GQSelector(const bool ofType);

		GQSelector(const int a, const int b, const bool last, const bool ofType);

		GQSelector(GumboTag tag);

		virtual ~GQSelector();

		virtual const bool Match(const GumboNode* node) const;

		std::vector<GumboNode*> MatchAll(const GumboNode* node) const;

		void Filter(std::vector<GumboNode*>& nodes) const;

	private:
		
		SelectorOperator m_selectorOperator = SelectorOperator::Dummy;

		int m_a = 0;
		int m_b = 0;
		bool m_last;
		bool m_ofType;
		GumboTag m_tag;

		void InitDefaults();

		void MatchAllInto(GumboNode* node, std::vector<GumboNode*>& nodes);

	};

	typedef std::shared_ptr<GQSelector> SharedGQSelector;

} /* namespace gumboquery */
