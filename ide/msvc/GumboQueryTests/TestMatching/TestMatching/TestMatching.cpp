#include <fstream>
#include <iostream>
#include <sstream>
#include <GQDocument.hpp>
#include <GQNode.hpp>

/// <summary>
/// The purpose of this test is to load the "matchingtest.data" data file and run the tests laid out
/// in that file, checking for failures. The "matchingtest.data" file contains a series of
/// predefined selectors and html to test the selectors against.
/// </summary>
int main()
{

	std::ifstream in(u8"C:\\Github\\GumboQuery\\test\\matchingtest.data", std::ios::in);
	//std::ifstream in(u8"../../matchingtest.data", std::ios::in);

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

	if (testNumbers.size() == testSelectors.size() && testExpectedMatches.size() == testNumbers.size() && testHtmlSamples.size() == testExpectedMatches.size())
	{

		for (size_t i = 0; i < testNumbers.size(); ++i)
		{
			gq::GQDocument document;

			document.Parse(testHtmlSamples[i]);

			try
			{
				std::cout << std::endl;
				auto result = document.Find(testSelectors[i]);

				if (result.GetNodeCount() != testExpectedMatches[i].second)
				{
					std::cout << u8"Test Number " << testNumbers[i] << u8" failed using selector " << testSelectors[i] << u8" because " << testExpectedMatches[i].second << u8" matches were expected, received " << result.GetNodeCount() << std::endl;
					++testsFailed;
					continue;
				}
				else
				{
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

