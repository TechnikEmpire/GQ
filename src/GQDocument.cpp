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

#include "GQDocument.hpp"

namespace gumboquery
{

	GQDocument::GQDocument()
	{
	}

	GQDocument::GQDocument(GumboOutput* gumboOutput)
	{
		// No attempting to construct explicitly supplying a pointer, when its null.
		assert(gumboOutput != nullptr && u8"In GQDocument::GQDocument(GumboOutput*) - Supplied GumboOutput* is nulltr! Use the parameterless constructor.");

		m_gumboOutput.reset(gumboOutput);
		m_rootSelection.reset(new GQSelection(m_gumboOutput->root));
	}

	GQDocument::~GQDocument()
	{
		if (m_gumboOutput != nullptr)
		{
			gumbo_destroy_output(&kGumboDefaultOptions, m_gumboOutput.release());
		}
	}

	void GQDocument::Parse(const std::string& source)
	{
		// No attempting to parse empty strings.
		assert(source.length() > 0 && (source.find_first_not_of(u8" \t\r\n") != std::string::npos) && u8"In GQDocument::Parse(const std::string&) - Empty or whitespace string supplied.");

		if (m_gumboOutput != nullptr)
		{
			gumbo_destroy_output(&kGumboDefaultOptions, m_gumboOutput.release());
		}

		GumboOutput* output = gumbo_parse(source.c_str());

		if (output == nullptr)
		{
			throw new std::runtime_error(u8"In GQDocument::Parse(const std::string&) - Failed to parse and or allocate GumboOutput.");
		}

		m_gumboOutput.reset(output);

		m_rootSelection.reset(new GQSelection(output->root));
	}

	GQSelection GQDocument::Find(const std::string& selectorString) const
	{
		if (m_gumboOutput == nullptr)
		{
			throw new std::runtime_error(u8"In GQDocument::Find(const std::string&) - Document is not initialized. You must parse an HTML string, or construct this object around a valid GumboOutput pointer.");
		}	

		if (m_rootSelection == nullptr)
		{
			// This shouldn't happen, but perhaps somehow when the object was last created, something amiss took place.
			throw new std::runtime_error(u8"In GQDocument::Find(const std::string&) - Root GQSelection is nullptr.");
		}
	}

	GQSelection GQDocument::Find(const GQSelector& selector) const
	{
		if (m_gumboOutput == nullptr)
		{
			throw new std::runtime_error(u8"In GQDocument::Find(const GQSelector&) - Document is not initialized. You must parse an HTML string, or construct this object around a valid GumboOutput pointer.");
		}

		if (m_rootSelection == nullptr)
		{
			// This shouldn't happen, but perhaps somehow when the object was last created, something amiss took place.
			throw new std::runtime_error(u8"In GQDocument::Find(const std::string&) - Root GQSelection is nullptr.");
		}
	}

} /* namespace gumboquery */
