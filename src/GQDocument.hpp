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

#include <cassert>
#include <memory>
#include "GQSelection.hpp"

namespace gumboquery
{

	/// <summary>
	/// The GQDocument serves as a lightweight wrapper around a GumboOutput* object, providing
	/// methods for querying the parsed HTML with selectors.
	/// </summary>
	class GQDocument
	{

	public:

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
		/// Default destructor.
		/// </summary>
		~GQDocument();

		/// <summary>
		/// Use Gumbo Parser internally to parse the supplied HTML string into GumboOutput. It is
		/// the responsibility of the user to ensure that the supplied HTML string is UTF-8 encoded,
		/// as Gumbo Parser requires this.
		/// </summary>
		/// <param name="source">
		/// A UTF-8 encoded string of a valid HTML. 
		/// </param>
		void Parse(const std::string& source);

		/// <summary>
		/// Run a selector against the document and return any and all nodes that were matched by
		/// the supplied selector string.
		/// </summary>
		/// <param name="selectorString">
		/// The selector string to query against the document with. 
		/// </param>
		/// <returns>
		/// A collection of nodes that were matched by the supplied selector. If no matches were
		/// found, the collection will be empty.
		/// </returns>
		GQSelection Find(const std::string& selectorString) const;

		/// <summary>
		/// Run a selector against the document and return any and all nodes that were matched by
		/// the supplied selector string.
		/// </summary>
		/// <param name="selector">
		/// The precompiled selector object to query against the document with. 
		/// </param>
		/// <returns>
		/// A collection of nodes that were matched by the supplied selector. If no matches were
		/// found, the collection will be empty.
		/// </returns>
		GQSelection Find(const GQSelector& selector) const;		

	private:

		/// <summary>
		/// unique_ptr wrapper for the raw GumboOutput pointer. This object exclusively holds
		/// ownership of the raw GumboOutput structure it works with.
		/// </summary>
		std::unique_ptr<GumboOutput> m_gumboOutput = nullptr;

		/// <summary>
		/// unique_ptr of the GQSelection instance which wraps m_gumboOutput-&gt;root. This should
		/// be reset every time m_gumboOutput is reset.
		/// </summary>
		std::unique_ptr<GQSelection> m_rootSelection = nullptr;
	};

} /* namespace gumboquery */
