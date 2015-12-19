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

#include <cassert>
#include <memory>
#include "GQSelection.hpp"

namespace gq
{

	class GQTreeMap;

	/// <summary>
	/// The GQDocument serves as a lightweight wrapper around a GumboOutput* object, providing
	/// methods for querying the parsed HTML with selectors.
	/// </summary>
	class GQDocument : public GQNode
	{

	public:		

		// If MSVC, must friend this nonsense so that make_shared can access the private constructor
		// of our class. If not, just friend the template function.
		#ifdef _MSC_VER
		friend std::_Ref_count_obj<GQDocument>;
		#else
		friend std::shared_ptr<GQNode> std::make_shared<>(const GumboNode*);
		#endif

		static std::shared_ptr<GQDocument> Create(GumboOutput* gumboOutput = nullptr);

		/// <summary>
		/// Default destructor.
		/// </summary>
		virtual ~GQDocument();

		/// <summary>
		/// Use Gumbo Parser internally to parse the supplied HTML string into GumboOutput. It is
		/// the responsibility of the user to ensure that the supplied HTML string is UTF-8 encoded,
		/// as Gumbo Parser requires this.
		/// </summary>
		/// <param name="source">
		/// A UTF-8 encoded string of a valid HTML. 
		/// </param>
		void Parse(const std::string& source);

	protected:

		/// <summary>
		/// Same concept as GQNode::Init(), which this overrides.
		/// </summary>
		virtual void Init();

	private:

		/// <summary>
		/// Constructs an empty GQDocument object.
		/// </summary>
		GQDocument();

		/// <summary>
		/// Constructs a GQDocument from existing GumboOuput. This object will assume ownership of
		/// the supplied of the GumboOutput pointer.
		/// </summary>
		/// <param name="gumboOutput">
		/// Existing output generated from Gumbo Parser. This object will assume control over the
		/// lifetime of the supplied GumboOutput.
		/// </param>
		GQDocument(GumboOutput* gumboOutput);

		/// <summary>
		/// A pointer to the GumboOutput generated when parsing HTML. This object exclusively holds
		/// ownership of the raw GumboOutput structure it works with.
		/// </summary>
		GumboOutput* m_gumboOutput = nullptr;

		/// <summary>
		/// The map for the entire document.
		/// </summary>
		std::unique_ptr<GQTreeMap> m_treeMap = nullptr;

	};

} /* namespace gq */
