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
#include "Selection.hpp"
#include "TreeMap.hpp"

namespace gq
{

	class TreeMap;

	/// <summary>
	/// The Document serves as a lightweight wrapper around a GumboOutput* object, providing
	/// methods for querying the parsed HTML with selectors.
	/// </summary>
	class Document : public Node
	{

	public:		

		static std::unique_ptr<Document> Create(GumboOutput* gumboOutput = nullptr);

		/// <summary>
		/// Default destructor.
		/// </summary>
		virtual ~Document();

		/// <summary>
		/// Use Gumbo Parser internally to parse the supplied HTML string into GumboOutput. It is
		/// the responsibility of the user to ensure that the supplied HTML string is UTF-8 encoded,
		/// as Gumbo Parser requires this.
		/// </summary>
		/// <param name="source">
		/// A UTF-8 encoded string of a valid HTML. 
		/// </param>
		void Parse(const std::string& source);		

	private:		

		/// <summary>
		/// Constructs an empty Document object.
		/// </summary>
		Document();

		/// <summary>
		/// Constructs a Document from existing GumboOuput. This object will assume ownership of
		/// the supplied of the GumboOutput pointer.
		/// </summary>
		/// <param name="gumboOutput">
		/// Existing output generated from Gumbo Parser. This object will assume control over the
		/// lifetime of the supplied GumboOutput.
		/// </param>
		Document(GumboOutput* gumboOutput);

		/// <summary>
		/// A pointer to the GumboOutput generated when parsing HTML. This object exclusively holds
		/// ownership of the raw GumboOutput structure it works with.
		/// </summary>
		GumboOutput* m_gumboOutput = nullptr;

		/// <summary>
		/// The map for the entire document.
		/// </summary>
		TreeMap m_treeMap;
		//std::unique_ptr<TreeMap> m_treeMap = nullptr;

		/// <summary>
		/// Same concept as Node::Init(), which this overrides.
		/// </summary>
		void Init();

	};

} /* namespace gq */
