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
#include <GQDocument.hpp>
#include <GQNode.hpp>
#include <GQParser.hpp>
#include <GQSerializer.hpp>

/// <summary>
/// The purpose of this test is to load the "matchingtest.data" data file and run the tests laid out
/// in that file, checking for failures. The "matchingtest.data" file contains a series of
/// predefined selectors and html to test the selectors against.
/// 
/// Run this test as well as GQ in debug mode to get an extreme amount of information about the
/// internals of the parser, selector construction, etc. To get this additional information, ensure
/// that while compiling GQ in debug mode, you add "GQ_VERBOSE_DEBUG_NFO" to the preprocessor
/// definitions. The verbose output can help gain much insight when debugging selectors and
/// the internals of GQ.
/// </summary>
int main()
{
	//std::string matchingTestDataFilePath(u8"../../matchingtest.data");
	std::string matchingTestDataFilePath(u8"C:\\Github\\GumboQuery\\test\\matchingtest.data");

	std::ifstream in(matchingTestDataFilePath, std::ios::binary | std::ios::in);

	if (in.fail())
	{
		std::cout << u8"Failed to load \"../../matchingtest.data\" test file." << std::endl;
		return -1;
	}

	std::vector<int> testNumbers;
	std::vector<std::string> testSelectors;
	std::vector< std::pair<bool, int> > testExpectedMatches;
	std::vector<std::string> testHtmlSamples;

	std::string testContents;
	in.seekg(0, std::ios::end);
	testContents.resize(in.tellg());
	in.seekg(0, std::ios::beg);
	in.read(&testContents[0], testContents.size());
	in.close();

	std::istringstream tests(testContents);
	std::string test;
	while (std::getline(tests, test))
	{
		if (test.length() == 0 || test[0] == '!')
		{
			// Skip empty lines and comments
			continue;
		}

		std::istringstream isstest(test);
		std::string testPart;
		while (std::getline(isstest, testPart, '%'))
		{
			auto splitPos = testPart.find('@');
			auto testVariable = testPart.substr(0, splitPos);
			auto testVariableValue = testPart.substr(splitPos + 1);

			if (testVariable.length() == 0 || testVariableValue.length() == 0)
			{
				std::cout << u8"Empty test variable or value encountered. Test file is improperly formatted. Aborting." << std::endl;
				return -1;
			}

			if (testVariable.compare(u8"TestNumber") == 0)
			{
				testNumbers.push_back(std::stoi(testVariableValue));
			}
			else if (testVariable.compare(u8"TestSelector") == 0)
			{
				testSelectors.push_back(testVariableValue);
			}
			else if (testVariable.compare(u8"TestExpectedMatches") == 0)
			{
				testExpectedMatches.push_back({ true, std::stoi(testVariableValue) });
			}
			else if (testVariable.compare(u8"TestExpectedUncheckedMatches") == 0)
			{
				testExpectedMatches.push_back({ false, std::stoi(testVariableValue) });
			}
			else if (testVariable.compare(u8"TestHtml") == 0)
			{
				testHtmlSamples.push_back(testVariableValue);
			}
		}
	}
	
	size_t testsPassed = 0;
	size_t testsFailed = 0;

	gq::GQParser parser;

	if (testNumbers.size() == testSelectors.size() && testExpectedMatches.size() == testNumbers.size() && testHtmlSamples.size() == testExpectedMatches.size())
	{

		for (size_t i = 0; i < testNumbers.size(); ++i)
		{
			for (size_t b = 0; b < 72; ++b)
			{
				std::cout << '-';
			}
			std::cout << std::endl << u8"\t\t\t\tTest #" << testNumbers[i] << std::endl;
			for (size_t b = 0; b < 72; ++b)
			{
				std::cout << '-';
			}
			std::cout << std::endl << std::endl;

			auto document = gq::GQDocument::Create();

			std::cout << u8"Input HTML:" << std::endl;
			std::cout << testHtmlSamples[i] << std::endl << std::endl;

			document->Parse(testHtmlSamples[i]);
			
			std::cout << u8"Parsed Output HTML:" << std::endl;
			std::cout << document->GetOuterHtml();

			try
			{
				std::cout << std::endl;
				
				auto selector = parser.CreateSelector(testSelectors[i], true);
				auto result = document->Find(selector);				
				std::cout << u8"Original Selector String: " << selector->GetOriginalSelectorString() << std::endl << std::endl;

				if (result.GetNodeCount() != testExpectedMatches[i].second)
				{
					std::cout << u8"Test Number " << testNumbers[i] << u8" failed using selector " << testSelectors[i] << u8" because " << testExpectedMatches[i].second << u8" matches were expected, received " << result.GetNodeCount() << std::endl << std::endl;
					++testsFailed;
					for (size_t ri = 0; ri < result.GetNodeCount(); ++ri)
					{
						auto node = result.GetNodeAt(ri);
						std::cout << gq::GQSerializer::Serialize(node.get()) << std::endl;
					}
					continue;
				}
				else
				{
					// If the test pair bool is true, that means that we're not just counting results, but that we're
					// also validating the results. We'll check the results to see if they contain the test "FAIL". If 
					// they do, that means that an element that should not have been selected by the selector was indeed
					// selected. If "FAIL" is not matched, "PASS" is assumed.
					if (testExpectedMatches[i].first == true)
					{
						bool foundInvalidData = false;
						for (size_t ri = 0; ri < result.GetNodeCount(); ++ri)
						{
							auto node = result.GetNodeAt(ri);
							if (node->GetOwnText().compare("FAIL") == 0)
							{								
								foundInvalidData = true;
								break;
							}

							std::cout << gq::GQSerializer::Serialize(node.get()) << std::endl;
						}

						if (foundInvalidData)
						{
							std::cout << u8"Test Number " << testNumbers[i] << u8" failed using selector " << testSelectors[i] << u8" because although the number of expected matches was accurate, the selector matched a node it should not have." << std::endl;
							++testsFailed;
						}
						else
						{
							std::cout << u8"Test Number " << testNumbers[i] << u8" passed using selector " << testSelectors[i] << u8" because the correct number of expected matches were returned the match data was confirmed." << std::endl;
							++testsPassed;
						}						
					}
					else
					{
						std::cout << u8"Test Number " << testNumbers[i] << u8" passed using selector " << testSelectors[i] << u8" because " << testExpectedMatches[i].second << u8" matches were expected, received " << result.GetNodeCount() << u8". Test does not verify results, only quantity." << std::endl;
						++testsPassed;
					}					
				}
			}
			catch (std::runtime_error& e)
			{
				std::cout << u8"Got runtime_error: " << e.what() << std::endl;
			}
			catch (std::exception& e)
			{
				std::cout << u8"Got exception: " << e.what() << std::endl;
			}		
		}		
	}
	else
	{
		std::cout << u8"An unequal number of test variables were parsed. Test file is improperly formatted. Aborting." << std::endl;
		return -1;
	}

	std::cout << testsPassed << u8" Tests Passed and " << testsFailed << u8" Tests Failed." << std::endl;

    return 0;
}

