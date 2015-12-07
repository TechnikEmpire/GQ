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
	std::ifstream in(u8"../../parsingtest.data", std::ios::in);

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
	while (std::getline(tests, test))
	{
		if (test.length() == 0 || test[0] == '!')
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
			std::cout << u8"Failed to locate the test number and or the test selector. The test data is improperly formatted. Aboring." << std::endl;
			return -1;
		}			

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

	std::cout << "Processed " << totalSelectorsProcessed << " selectors. Had handled errors? " << anyHandledException << std::endl;

    return 0;
}

