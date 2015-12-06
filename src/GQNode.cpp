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

#include "GQNode.hpp"
#include "GQUtil.hpp"
#include "GQSelection.hpp"

namespace gq
{

	std::shared_ptr<GQNode> GQNode::Create(const GumboNode* node)
	{
		return std::make_shared<GQNode>(node);
	}

	GQNode::GQNode(const GumboNode* node) : 
		m_node(node)
	{		
		#ifndef NDEBUG
		assert(node != nullptr && u8"In GQNode::GQNode(const GumboNode*) - Cannot construct a GQNode around a nullptr.");
		#else
		if (node == nullptr) { throw std::runtime_error(u8"In GQNode::GQNode(const GumboNode*) - Cannot construct a GQNode around a nullptr."); }
		#endif

		/*
		if (node->parent != nullptr)
		{
			m_sharedParent = Create(node->parent);

			// Todo - construct previous and next siblings.
			if (node->parent->v.element.children.length > 1)
			{
				if (node->index_within_parent == 0)
				{
					m_sharedNextSibling = Create(static_cast<GumboNode*>(node->parent->v.element.children.data[1]));
				}
				else if (node->index_within_parent == (node->parent->v.element.children.length - 1))
				{
					m_sharedPreviousSibling = Create(static_cast<GumboNode*>(node->parent->v.element.children.data[node->index_within_parent - 1]));
				}
				else
				{
					// This can only be possible if the number of children is greater than 2 and this node has
					// a valid sibling on either side.
					m_sharedNextSibling = Create(static_cast<GumboNode*>(node->parent->v.element.children.data[node->index_within_parent + 1]));
					m_sharedPreviousSibling = Create(static_cast<GumboNode*>(node->parent->v.element.children.data[node->index_within_parent - 1]));
				}
			}
		}
		*/
	}

	GQNode::~GQNode()
	{

	}

	SharedGQNode GQNode::GetParent() const
	{
		return m_sharedParent;
	}

	const size_t GQNode::GetIndexWithinParent() const
	{
		return m_node->index_within_parent;
	}

	SharedGQNode GQNode::GetPreviousSibling() const
	{
		return m_sharedPreviousSibling;
	}

	SharedGQNode GQNode::GetNextSibling() const
	{
		return m_sharedNextSibling;
	}

	const size_t GQNode::GetNumChildren() const
	{
		if (m_node->type != GUMBO_NODE_ELEMENT)
		{
			return 0;
		}

		return m_node->v.element.children.length;
	}

	SharedGQNode GQNode::GetChildAt(const size_t position) const
	{
		if (m_node->type != GUMBO_NODE_ELEMENT || position >= m_node->v.element.children.length)
		{
			throw std::runtime_error(u8"In GQNode::GetChildAt(const size_t) - Supplied index is out of bounds.");
		}

		return Create(static_cast<GumboNode*>(m_node->v.element.children.data[position]));
	}

	std::string GQNode::GetAttributeValue(const std::string& attributeName) const
	{	
		boost::string_ref attributeNameStringRef(attributeName);

		auto result = GetAttributeValue(attributeNameStringRef);

		return result.to_string();
	}

	boost::string_ref GQNode::GetAttributeValue(const boost::string_ref& attributeName) const
	{	
		boost::string_ref result;

		if (m_node->type != GUMBO_NODE_ELEMENT)
		{
			return result;
		}

		const GumboVector* attributes = &m_node->v.element.attributes;

		for (unsigned int i = 0; i < attributes->length; i++)
		{
			GumboAttribute* attr = static_cast<GumboAttribute*>(attributes->data[i]);

			boost::string_ref attrName(attr->name);

			if (boost::iequals(attributeName, attrName))
			{
				result = boost::string_ref(attr->value);
			}
		}

		return result;
	}

	std::string GQNode::GetText() const
	{
		return GQUtil::NodeText(m_node);
	}

	std::string GQNode::GetOwnText() const
	{
		return GQUtil::NodeOwnText(m_node);
	}

	const size_t GQNode::GetStartPosition() const
	{
		switch (m_node->type)
		{
			case GUMBO_NODE_ELEMENT:
			{				
				return m_node->v.element.start_pos.offset + m_node->v.element.original_tag.length;
			}
			break;

			case GUMBO_NODE_TEXT:
			{
				return m_node->v.text.start_pos.offset;
			}
			break;

			default:
				// XXX TODO - I don't like the idea of returning zero since it implies a valid position.
				// Perhaps change the return values to be int32_t and return -1 for invalid nodes. However,
				// matching shouldn't work against non element and non text nodes, so there theoretically
				// could never be a situation where this code is reached. For now this is to shut up the
				// compiler.
				return 0;
		}
	}

	const size_t GQNode::GetEndPosition() const
	{
		switch (m_node->type)
		{
			case GUMBO_NODE_ELEMENT:
			{
				return m_node->v.element.end_pos.offset;
			}
			break;

			case GUMBO_NODE_TEXT:
			{
				return m_node->v.text.original_text.length + GetStartPosition();
			}
			break;

			default:
				// XXX TODO - I don't like the idea of returning zero since it implies a valid position.
				// Perhaps change the return values to be int32_t and return -1 for invalid nodes. However,
				// matching shouldn't work against non element and non text nodes, so there theoretically
				// could never be a situation where this code is reached. For now this is to shut up the
				// compiler.
				return 0;
		}
	}

	const size_t GQNode::GetStartOuterPosition() const
	{
		switch (m_node->type)
		{
			case GUMBO_NODE_ELEMENT:
			{
				return m_node->v.element.start_pos.offset;
			}
			break;

			case GUMBO_NODE_TEXT:
			{
				return m_node->v.text.start_pos.offset;
			}
			break;

			default:
				// XXX TODO - I don't like the idea of returning zero since it implies a valid position.
				// Perhaps change the return values to be int32_t and return -1 for invalid nodes. However,
				// matching shouldn't work against non element and non text nodes, so there theoretically
				// could never be a situation where this code is reached. For now this is to shut up the
				// compiler.
				return 0;
		}
	}

	const size_t GQNode::GetEndOuterPosition() const
	{
		switch (m_node->type)
		{
			case GUMBO_NODE_ELEMENT:
			{
				return m_node->v.element.end_pos.offset + m_node->v.element.original_end_tag.length;
			}
			break;

			case GUMBO_NODE_TEXT:
			{
				return m_node->v.text.original_text.length + GetStartPosition();
			}
			break;

			default:
				// XXX TODO - I don't like the idea of returning zero since it implies a valid position.
				// Perhaps change the return values to be int32_t and return -1 for invalid nodes. However,
				// matching shouldn't work against non element and non text nodes, so there theoretically
				// could never be a situation where this code is reached. For now this is to shut up the
				// compiler.
				return 0;
		}
	}

	std::string GQNode::GetTagName() const
	{	
		if (m_node->type != GUMBO_NODE_ELEMENT)
		{
			return "";
		}

		return std::string(gumbo_normalized_tagname(m_node->v.element.tag));
	}

	GumboTag GQNode::GetTag() const
	{
		if (m_node->type != GUMBO_NODE_ELEMENT)
		{
			return GumboTag(0);
		}

		return m_node->v.element.tag;
	}

	GQSelection GQNode::Find(const std::string& selector)
	{
		GQSelection selection(shared_from_this());

		return selection.Find(selector);
	}

	GQSelection GQNode::Find(const SharedGQSelector& selector)
	{
		GQSelection selection(shared_from_this());

		return selection.Find(selector);
	}

} /* namespace gq */

