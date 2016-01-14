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
#include <Document.hpp>
#include <Node.hpp>
#include <Parser.hpp>
#include <Serializer.hpp>
#include <chrono>
#include <cmath>

std::string BuildAttribute(boost::string_ref attrName, boost::string_ref attrValue)
{
	std::string ret(u8" ");
	if (attrValue.size() > 0)
	{
		ret.append(attrName.to_string() + u8"=\"" + attrValue.to_string() + u8"\" ");
	}
	else
	{
		ret.append(attrName.to_string() + u8" ");
	}

	return ret;
}

/// <summary>
/// This example shows basic usage of the mutation API. As you can see, it's simple, but offers a
/// great deal of power and flexibility to the end user.
/// </summary>
int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cout << u8"Usage: " << argv[0] << u8" PATH_TO_SOME_HTML_FILE" << std::endl;
		return -1;
	}

	std::string htmlTestDataFilePath(argv[1]);
	
	std::ifstream htmlFile(htmlTestDataFilePath, std::ios::binary | std::ios::in);

	if (htmlFile.fail() || !htmlFile.is_open())
	{
		std::cout << u8"Failed to load " << htmlTestDataFilePath << u8" test file." << std::endl;
		return -1;
	}	

	std::string testHtmlContents;
	htmlFile.seekg(0, std::ios::end);

	auto fsize = htmlFile.tellg();

	if (fsize < 0 || static_cast<unsigned long long>(fsize) > static_cast<unsigned long long>(std::numeric_limits<size_t>::max()))
	{
		std::cout << u8"When loading the input HTML file, ifstream::tellg() returned either less than zero or a number greater than this program can correctly handle." << std::endl;
		return -1;
	}

	testHtmlContents.resize(static_cast<size_t>(fsize));
	htmlFile.seekg(0, std::ios::beg);
	htmlFile.read(&testHtmlContents[0], testHtmlContents.size());
	htmlFile.close();

	auto testDocument = gq::Document::Create();

	try
	{
		testDocument->Parse(testHtmlContents);
	}
	catch (std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}	

	gq::NodeMutationCollection collection;	

	// OnTagStart allows us to choose whether or not allow a certain tag type matched by our
	// selector(s) to be serialized at all.
	collection.SetOnTagStart(
		[](const GumboTag tag)->bool
	{
		switch (tag)
		{
			case GumboTag::GUMBO_TAG_A:
			{
				// Let's just return false, which will omit this "a" tag from the final output.
				// Since we're saying "false" which means "don't serialize this", all children
				// of this node will be omitted from output as well.
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

	// SetOnTagAttribute allows us to have granular control over every attribute of nodes matched
	// by our selector(s).
	collection.SetOnTagAttribute(
		[](const GumboTag tag, std::string& tagString, boost::string_ref attributeName, boost::string_ref attributeValue)->void
	{
		switch (tag)
		{
			case GumboTag::GUMBO_TAG_IMAGE:
			{
				// Let's make all images point to something super spoopy.
				if (attributeName.compare(u8"src") == 0)
				{
				
					tagString.append(BuildAttribute(attributeName, boost::string_ref(u8"https://i.ytimg.com/vi/dY_h3q6vgmY/maxresdefault.jpg")));
				}
				else
				{
					tagString.append(BuildAttribute(attributeName, attributeValue));
				}
				return;
			}
			break;

			case GumboTag::GUMBO_TAG_IFRAME:
			{
				// Let's make all iframes go somewhere that no man has gone before.
				if (attributeName.compare(u8"src") == 0)
				{
					tagString.append(BuildAttribute(attributeName, boost::string_ref(u8"http://www.startrek.com/")));
				}
				else
				{
					tagString.append(BuildAttribute(attributeName, attributeValue));
				}
				return;
			}
			break;

			default:
			{
				// Let's keep everything else without modification.
				tagString.append(BuildAttribute(attributeName, attributeValue));
			}
			break;
		}
	});

	// SetOnTagAttribute allows us to have control over the content of each node matched by
	// our selector(s).
	collection.SetOnTagContent(
		[](const GumboTag tag, std::string& tagString)->bool
	{
		switch (tag)
		{
			case GumboTag::GUMBO_TAG_P:
			{
				// Make all paragraph tags contain our custom text, while still serializing any non-text contents
				// correctly. We return true, as this will instruct the serializer to replace only the plain text
				// content of the node with the content we push to the supplied string. In other words, by returning
				// true, all GUMBO_TAG_TEXT nodes are omitted and our string replaces them all, while all other types
				// of gumbo nodes which are children of this node are serialized normally.
				//
				// If we wanted to replace the entire contents with whatever we push to the string, we'd return false.

				tagString.append(u8"There might have been some other text here, but now, it's only ours.");

				// We can also make up our own html to insert within the node.
				//tagString.append(u8"<p><a href=\"http://somelink.com\">There might have been some other text here, but now, it's only ours.</a></p>");
				return true;
			}
			break;

			case GumboTag::GUMBO_TAG_DIV:
			{
				// Inject a link within a paragraph as the first child. Since we return "true", which again means to
				// replace any text, if any, this will get appended first, then the rest of the existing children
				// will be serialized.
				tagString.append(u8"<p><a href=\"http://somelink.com\">Can you div it?</a></p>");
				return true;
			}
			break;

			default:
			{
				// Since we're not appending any data to the supplied string, we can simply return true
				// or false, doesn't matter. By not adding any data, we're telling the serializer to
				// continue without our intervention.
				//return true;
				return false;
			}
			break;
		}
	});
	
	// Create a parser to build a selector from a string/
	gq::Parser selectorParser;

	// Create our selector string, in this case a combined selector that will
	// match all div, p, a, and iframe elements in the html.
	std::string selectorString(u8"div, p, a, iframe");

	gq::SharedSelector selector = nullptr;

	try
	{
		// Wrap our selector parsing in a try/catch, since it goes without
		// saying that parsing user input can definitely throw.
		selector = selectorParser.CreateSelector(selectorString);	

		if (selector == nullptr)
		{
			// Shouldn't happen ever, but better safe than sorry.
			return -1;
		}
	}
	catch (std::runtime_error& error)
	{
		// Most surely an improperly formatted selector.
		std::cout << error.what() << std::endl;
		return -1;
	}
	
	// Tell the document to give us every node that matches our selector in a callback.
	// There may or may not be duplicates given to us. We don't really care too much
	// about that here.
	testDocument->Each(selector,
		[&collection](const gq::Node* node)->void
	{
		collection.Add(node);
	});

	// Give our mutation collection to the serialize method and get the serialization result.
	auto serialized = gq::Serializer::Serialize(testDocument.get(), &collection);

	std::string finalResult;

	auto docStartPos = testDocument->GetStartOuterPosition();
	auto docEndPos = testDocument->GetEndOuterPosition();

	// We might have got some valid HTML that was embedded in some other unknown data. So, we'll
	// check the doc->GetStartOuterPosition() and doc->GetEndOuterPosition(), then copy the
	// difference in offsets onto the serialized string and return it. This way, we're not blowing
	// away any data we shouldn't be.
	if (docStartPos > 0)
	{
		finalResult.append(testHtmlContents.substr(0, docStartPos));
	}

	finalResult.append(serialized);

	if (docEndPos < (testHtmlContents.size() - 1))
	{
		finalResult.append(testHtmlContents.substr(docEndPos + 1));
	}

	std::string outputFileName = htmlTestDataFilePath;
	outputFileName.append(u8".filtered.html");

	std::ofstream output(outputFileName, std::ios::binary | std::ios::out);

	if (output.fail() || !output.is_open())
	{
		std::cout << u8"Failed to open output file " << outputFileName << u8" for saving." << std::endl;
		return -1;
	}

	// Write result to file.
	output << finalResult;

	output.close();

	// Fin.
    return 0;
}

