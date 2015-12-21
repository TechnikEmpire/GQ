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

#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <cmath>
#include <GQDocument.hpp>
#include <GQNode.hpp>
#include <GQParser.hpp>
#include <GQNodeMutationCollection.hpp>
#include <GQSerializer.hpp>

/// <summary>
/// The purpose of this test is threefold. One, load the "parsingtest.data" list of selectors and
/// parse them all, ensuring that no errors are thrown while parsing them.
/// 
/// The second is to, in the event that no errors are thrown, benchmark parsing the list just to get
/// a sense of the speed of the parser when dealing with "in the wild" data. This isn't really
/// necessary to test, since GQ allows caching/saving of final compiled selectors. But hey, why not.
/// 
/// The third is to, again in the event of no errors on first pass, load the HTML from a given
/// website and benchmark running all of the selectors against it, to get an idea of speed on
/// matching against "in the wild" data.
/// </summary>
int main()
{

	std::string parsingTestDataFilePath(u8"../../parsingtest.data");
	std::string htmlTestDataFilePath(u8"../../testhtml.data");
	
	std::ifstream in(parsingTestDataFilePath, std::ios::binary | std::ios::in);

	if (in.fail())
	{
		std::cout << u8"Failed to load \"../../parsingtest.data\" test file." << std::endl;
		return -1;
	}
		
	std::string testContents;
	in.seekg(0, std::ios::end);
	testContents.resize(in.tellg());
	in.seekg(0, std::ios::beg);
	in.read(&testContents[0], testContents.size());
	in.close();

	gq::GQParser selectorParser;

	bool anyHandledException = false;
	size_t totalSelectorsProcessed = 0;

	std::istringstream tests(testContents);
	std::string test;

	// For finding out just where the program died if an attempt to parse a test number
	// and selector fails.
	int lastRunTestNumber = 0;

	std::vector<std::string> selectors;

	while (std::getline(tests, test))
	{
		if (test.size() == 0 || test[0] == '!')
		{
			// Skip empty lines and comments
			continue;
		}

		std::string selectorString;
		int testNumber;
		try
		{
			selectorString = test.substr(test.find_last_of("@") + 1);
			auto testNumberStart = test.find("@");
			auto testNumberEnd = test.find("%");
			testNumber = std::stoi(test.substr(testNumberStart + 1, (testNumberStart + 1) - testNumberEnd));
		}
		catch (...)
		{
			std::cout << u8"Failed to locate the test number and or the test selector after test " << lastRunTestNumber << u8". The test data is improperly formatted. Aboring." << std::endl;
			return -1;
		}			

		// Save the last finished test number in case the above try block fails and we exit due to bad formatting
		// of the test data.
		lastRunTestNumber = testNumber;

		// Push back selectors anticipating doing benchmarks.
		selectors.push_back(selectorString);

		try
		{
			auto result = selectorParser.CreateSelector(selectorString);
			++totalSelectorsProcessed;
		}
		catch (std::runtime_error& e)
		{
			anyHandledException = true;
			std::cout << std::endl;
			std::cout << u8"In test number " << testNumber << u8" using selector string " << selectorString << u8" got runtime_error: " << e.what() << std::endl;			
		}
		catch (std::exception& e)
		{
			anyHandledException = true;
			std::cout << std::endl;
			std::cout << u8"In test number " << testNumber << u8" using selector string " << selectorString << u8" got exception: " << e.what() << std::endl;
		}	
	}

	std::cout << u8"Processed " << totalSelectorsProcessed << u8" selectors. Had handled errors? " << std::boolalpha << anyHandledException << std::endl;

	if (anyHandledException)
	{
		std::cout << u8"Aborting benchmarks because errors were detected in the initial parsing test." << std::endl;
		return -1;
	}
	
	std::cout << u8"Benchmarking parsing speed." << std::endl;

	size_t parseCount = 100;

	auto parsingBenchStart = std::chrono::high_resolution_clock::now();

	for (size_t t = 0; t < parseCount; ++t)
	{
		for (size_t i = 0; i < selectors.size(); ++i)
		{
			auto result = selectorParser.CreateSelector(selectors[i]);
		}
	}

	auto parsingBenchEnd = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> parsingBenchTime = parsingBenchEnd - parsingBenchStart;

	std::cout << "Time taken to parse " << (selectors.size() * parseCount) << u8" selectors: " << parsingBenchTime.count() << " ms." << std::endl;

	std::cout << "Processed at a rate of " << (parsingBenchTime.count() / (selectors.size() * parseCount)) << u8" milliseconds per selector or " << ((selectors.size() * parseCount) / parsingBenchTime.count()) << u8" selectors per millisecond." << std::endl;

	// _______________________________________________________________________________________________ //
	// _______________________________________________________________________________________________ //
	
	std::ifstream htmlFile(htmlTestDataFilePath, std::ios::binary | std::ios::in);

	if (htmlFile.fail())
	{
		std::cout << u8"Failed to load \"../../testhtml.data\" test file." << std::endl;
		return -1;
	}	

	std::string testHtmlContents;
	htmlFile.seekg(0, std::ios::end);
	testHtmlContents.resize(htmlFile.tellg());
	htmlFile.seekg(0, std::ios::beg);
	htmlFile.read(&testHtmlContents[0], testHtmlContents.size());
	htmlFile.close();

	std::vector<gq::SharedGQSelector> precompiledSelectors;
	precompiledSelectors.reserve(selectors.size());

	for (size_t ind = 0; ind < selectors.size(); ++ind)
	{

		auto result = selectorParser.CreateSelector(selectors[ind], true);
		precompiledSelectors.push_back(result);
	}

	std::cout << u8"Benchmarking document parsing." << std::endl;

	size_t documentParseCount = 100;

	auto documentBuildStart = std::chrono::high_resolution_clock::now();

	for (size_t i = 0; i < documentParseCount; ++i)
	{
		auto doc = gq::GQDocument::Create();
		doc->Parse(testHtmlContents);
	}	

	auto documentBuildEnd = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> documentBuildTime = documentBuildEnd - documentBuildStart;
	
	std::cout << "Time taken to parse " << documentParseCount << u8" documents: " << documentBuildTime.count() << u8" ms." << std::endl;

	std::cout << "Processed at a rate of " << (documentBuildTime.count() / documentParseCount) << u8" milliseconds per document." << std::endl;

	std::cout << u8"Benchmarking selection speed." << std::endl;	

	auto testDocument = gq::GQDocument::Create();
	testDocument->Parse(testHtmlContents);

	size_t selectCount = 100;

	auto selectionBenchStart = std::chrono::high_resolution_clock::now();

	size_t totalMatches = 0;

	for (size_t si = 0; si < selectCount; ++si)
	{
		for (size_t i = 0; i < precompiledSelectors.size(); ++i)
		{
			auto selectionResults = testDocument->Find(precompiledSelectors[i]);
			
			#ifndef NDEBUG
			std::cout << "Total matches: " << selectionResults.GetNodeCount() << std::endl;
			#endif

			totalMatches += selectionResults.GetNodeCount();
		}
	}

	auto selectionBenchEnd = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> selectionBenchTime = selectionBenchEnd - selectionBenchStart;

	std::cout << "Time taken to run " << (precompiledSelectors.size() * selectCount) << u8" selectors against the document: " << selectionBenchTime.count() << u8" ms producing " << totalMatches << u8" total matches." << std::endl;

	std::cout << "Processed at a rate of " << (selectionBenchTime.count() / (precompiledSelectors.size() * selectCount)) << u8" milliseconds per selector or " << ((precompiledSelectors.size() * selectCount) / selectionBenchTime.count()) << u8" selectors per millisecond." << std::endl;	
    
	// _______________________________________________________________________________________________ //
	// _______________________________________________________________________________________________ //

	std::cout << u8"Benchmarking mutation." << std::endl;

	size_t mutateCount = 100;

	auto mutationBenchStart = std::chrono::high_resolution_clock::now();

	size_t totalBytesSerialized = 0;

	for (size_t si = 0; si < mutateCount; ++si)
	{
		gq::GQNodeMutationCollection collection;

		// OnTagStart allows us to choose whether or not allow a certain tag type matched by our
		// selector(s) to be serialized at all.
		collection.SetOnTagStart(
			[](const GumboTag tag)->bool
		{
			switch (tag)
			{
			case GumboTag::GUMBO_TAG_A:
			{
				// Let's just return without adding anything, which will omit this "a" tag from the 
				// final output. Since we do this without any other condition, all "a" tags that we
				// collected with our selector(s) will be omitted.
				return false;
			}
			break;

			case GumboTag::GUMBO_TAG_SCRIPT:
			{
				// Same deal as "a" tags.
				return false;
			}
			break;

			default:
			{
				// Let's keep everything else we selected.
				return true;
			}
			break;
			}
		});

		for (size_t i = 0; i < precompiledSelectors.size(); ++i)
		{
			testDocument->Each(precompiledSelectors[i],
				[&collection](const gq::GQNode* node)->void
			{
				collection.Add(node);
			});			
		}

		// Give our mutation collection to the serialize method and get the serialization result.
		auto serialized = gq::GQSerializer::Serialize(testDocument.get(), &collection);

		totalBytesSerialized += serialized.size();
	}

	auto mutationBenchEnd = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> mutationBenchTime = mutationBenchEnd - mutationBenchStart;

	std::cout << "Time taken to run " << precompiledSelectors.size() << u8" selectors against the document while serializing with mutations " << mutateCount << u8" times: " << mutationBenchTime.count() << u8" ms." << std::endl;
	std::cout << "Time per cycle " << (mutationBenchTime.count() / mutateCount) << u8" ms." << std::endl;
	std::cout << "Processed at a rate of " << (mutationBenchTime.count() / (precompiledSelectors.size() * mutateCount)) << u8" milliseconds per selector or " << ((precompiledSelectors.size() * mutateCount) / mutationBenchTime.count()) << u8" selectors per millisecond." << std::endl;

	return 0;
}

