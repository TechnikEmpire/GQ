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

#include "Document.hpp"
#include "Parser.hpp"
#include "Util.hpp"
#include "Serializer.hpp"

namespace gq
{
	std::unique_ptr<Document> Document::Create(GumboOutput* gumboOutput)
	{
		if (gumboOutput != nullptr)
		{
			auto doc = std::unique_ptr<Document>{ new Document(gumboOutput) };
			
			// Must call init to build out and index shared_ptr children.
			doc->Init();

			return doc;
		}

		return std::unique_ptr<Document>{ new Document() };
	}

	Document::Document()
	{
		m_parent = nullptr;
	}

	Document::Document(GumboOutput* gumboOutput) :
		m_gumboOutput(gumboOutput)
	{
		// No attempting to construct explicitly supplying a pointer, when its null. 
		#ifndef NDEBUG
			assert(gumboOutput != nullptr && u8"In Document::Document(GumboOutput*) - Supplied GumboOutput* is nulltr! Use the parameterless constructor.");
		#else
			if (gumboOutput == nullptr) { throw std::runtime_error(u8"In Document::Document(GumboOutput*) - Supplied GumboOutput* is nulltr! Use the parameterless constructor."); }
		#endif
		
		m_node = m_gumboOutput->root;
	}

	Document::~Document()
	{
		if (m_gumboOutput != nullptr)
		{
			gumbo_destroy_output(&kGumboDefaultOptions, m_gumboOutput);
		}
	}

	void Document::Parse(const std::string& source)
	{
		// No attempting to parse empty strings.
		#ifndef NDEBUG
			assert(source.length() > 0 && (source.find_first_not_of(u8" \t\r\n") != std::string::npos) && u8"In Document::Parse(const std::string&) - Empty or whitespace string supplied.");
		#else
			if (source.length() == 0 || (source.find_first_not_of(u8" \t\r\n\f") == std::string::npos)) { throw std::runtime_error(u8"In Document::Parse(const std::string&) - Empty or whitespace string supplied."); }
		#endif

		if (m_gumboOutput != nullptr)
		{
			gumbo_destroy_output(&kGumboDefaultOptions, m_gumboOutput);
		}

		m_gumboOutput = gumbo_parse(source.c_str());

		if (m_gumboOutput == nullptr)
		{
			throw std::runtime_error(u8"In Document::Parse(const std::string&) - Failed to parse and or allocate GumboOutput.");
		}

		m_node = m_gumboOutput->root;

		m_treeMap.Clear();

		// Must call init to build out and index and children children.
		Init();
	}

	void Document::Init()
	{
		m_nodeUniqueId = "0";
		m_indexWithinParent = 0;
		m_parent = nullptr;

		// Needed so that we don't have to override entire methods just to use a different pointer.
		// Yeah it's a little gross, this design. This is the product of suddenly drastically
		// changing the design to having Document subclass Node. XXX TODO find a more elegant
		// way to clean this up.
		m_rootTreeMap = &m_treeMap;

		BuildAttributes();
		BuildChildren();
	}

} /* namespace gq */
