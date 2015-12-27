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

#include "Util.hpp"
#include "Node.hpp"

namespace gq
{

	Util::Util()
	{

	}

	Util::~Util()
	{

	}

	std::string Util::NodeText(const Node* node)
	{
		std::string text;
		WriteNodeText(node->m_node, text);
		return text;
	}

	std::string Util::NodeOwnText(const Node* node)
	{
		std::string text;

		if (node->m_node == nullptr || node->m_node->type != GUMBO_NODE_ELEMENT)
		{
			return text;
		}

		const GumboVector* children = &node->m_node->v.element.children;
		for (unsigned int i = 0; i < children->length; i++)
		{
			GumboNode* child = static_cast<GumboNode*>(children->data[i]);
			if (child->type == GUMBO_NODE_TEXT)
			{
				text.append(child->v.text.text);
			}
		}

		return text;
	}

	bool Util::NodeExists(const std::vector< const Node* >& nodeCollection, const GumboNode* search)
	{
		if (search == nullptr)
		{
			return false;
		}

		return std::find_if(nodeCollection.begin(), nodeCollection.end(), 
			[search](const Node* item)
			{
				return item->m_node == search;
			}) != nodeCollection.end();
	}

	void Util::RemoveDuplicates(std::vector< const Node* >& primaryCollection)
	{
		std::sort(primaryCollection.begin(), primaryCollection.end(),
			[](const Node* lhs, const Node* rhs)
		{
			return lhs->m_node < rhs->m_node;
		}
		);

		auto last = std::unique(primaryCollection.begin(), primaryCollection.end(),
			[](const Node* lhs, const Node* rhs)
		{
			return lhs->m_node == rhs->m_node;
		}
		);

		primaryCollection.erase(last, primaryCollection.end());
	}

	void Util::UnionNodes(std::vector< const Node* >& primaryCollection, const std::vector< const Node* >& collection)
	{		
		primaryCollection.reserve(primaryCollection.size() + collection.size());
		primaryCollection.insert(primaryCollection.end(), collection.begin(), collection.end());
		
		RemoveDuplicates(primaryCollection);		
	}

	boost::string_ref Util::TrimEnclosingQuotes(boost::string_ref str)
	{
		if (str.length() >= 2)
		{
			switch (str[0])
			{
				case '\'':
				case '"':
				{
					if (str[str.length() - 1] == str[0])
					{
						size_t end = str.length() - 2;
						if (end == 0)
						{
							// Just so that it's not nullptr internall
							return boost::string_ref(str.data(), 0);
						}

						str = str.substr(1, end);
					}
				}
				break;

				default:
					break;
			}
		}

		return str;
	}

	void Util::WriteNodeText(const GumboNode* node, std::string& stringContainer)
	{
		if (node == nullptr)
		{
			return;
		}

		switch (node->type)
		{
			case GUMBO_NODE_TEXT:
			{
				stringContainer.append(node->v.text.text);
				break;
			}
			break;

			case GUMBO_NODE_ELEMENT:
			{
				const GumboVector* children = &node->v.element.children;
				for (unsigned int i = 0; i < children->length; i++)
				{
					GumboNode* child = static_cast<GumboNode*>(children->data[i]);
					WriteNodeText(child, stringContainer);
				}
			}
			break;

			default:
				break;
		}
	}

} /* namespace gq */