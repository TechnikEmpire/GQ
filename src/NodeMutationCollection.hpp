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

#pragma once

#include <unordered_set>
#include <functional>
#include <boost/utility/string_ref.hpp>
#include <gumbo.h>
#include <memory>
#include <string>

namespace gq
{

	class GQNode;

	/// <summary>
	/// The purpose of the GQNodeMutationCollection class is to provide a safe way for users to
	/// modify nodes during the serialization process. Users should not be concerned about validity
	/// of pointers in the collection or the lifetime of the objects in the collection. Once again,
	/// the user only must simply keep GQDocument alive and any naked pointers received by the user
	/// during its lifetime should be safe and valid, as they are managed internally this way. This
	/// collection also does not allow for removal. This collection internally uses an
	/// unordered_set, and as such sort of has built in duplicate filtering, but that's not the
	/// intended use and this behavior should not be relied upon.
	/// <para>&#160;</para>
	/// Gumbo Parser does not provide any way to mutate a parsed document. The first thought to
	/// solve this would be to provide lots of methods to fake the appearance of mutability, such as
	/// GQNode::SetText(...), copying data, then on serialization looking up all of these fake
	/// changes and attempting to place them correctly in the output. However, this could get
	/// complex fast, making maintenance a PITA.
	/// <para>&#160;</para>
	/// An alternative approach is to use a structure like this. When users really want to change
	/// nodes that are matched by selectors, they can store match results in this container and
	/// supply it to the serialization overload that accepts it. During the serialization process,
	/// one or more callbacks on the elements in the collection will be invoked, which will allow
	/// the user to write their own logic for changes, rather than taking on the burden in-project
	/// and forcing a "one-size-fits-all" on to all users.
	/// <para>&#160;</para>
	/// The reason why we need a specialized structure like this rather than an ordinary container
	/// is because of the extraordinary lengths taken by this library to prevent the end user from
	/// directly accessing underlying Gumbo structures that are managed by structures in this
	/// library, such as GumboNode. The second reason is that not every element in the GumboOutput
	/// is created and stored in a GQNode, because GQNodes are only constructed from
	/// GUMBO_NODE_ELEMENT nodes. The GQSerializer is agnostic about this design, and is agnostic
	/// about GQNode in general. We'd like to keep it that way (to keep it simple) and so we pass in
	/// this collection which is a friend of GQNode, and can, in a blackbox fashion (from the
	/// GQSerializer's perspective) handle recognizing a GumboNode* that's wrapped in a GQNode, etc
	/// etc.
	/// <para>&#160;</para>
	/// And just for the sake of explaining it to death, the user doesn't have access to the
	/// underlying GumboNode* elements. Since GQSerializer is largely agnostic to GQNode* and rather
	/// deals in raw GumboNode* structures, a gap needs to be bridged between the GQSerializer and
	/// the end user, who only deals in GQNode elements. But, we also don't want to be iterating
	/// over every GQNode the user has collected and wants to modify on serialization, comparing
	/// pointers in a repeating, recursive fashion (to match GumboNode* elements the serializer has
	/// discovered against GumboNode* elements the user is handling, wrapped up in GQNode*
	/// structures). So, this container is specialized so that GQSerializer can quickly ask it "is
	/// this next node I'm going to process something the user wants to modify?", and if that's
	/// true, GQSerializer will invoke the callback(s) also supplied by the user to ask the user's
	/// logic to perform serialization for that node, rather than by itself.
	/// </summary>
	class GQNodeMutationCollection
	{		
		friend class GQSerializer;

	public:

		using OnTagCallback = std::function<bool(const GumboTag tagType)>;
		using OnTagContentCallback = std::function<bool(const GumboTag tagType, std::string& tagString)>;
		using OnTagAttributeCallback = std::function<void(const GumboTag tagType, std::string& tagString, boost::string_ref attributeName, boost::string_ref attributeValue)>;

		/// <summary>
		/// Adds a node to the collection. Each node added to this collection will have its
		/// serialization delegated to the user.
		/// </summary>
		/// <param name="node">
		/// A node that requires custom serialization. 
		/// </param>
		void Add(const GQNode* node);

		/// <summary>
		/// Sets the callback to be used at the start of the serialization for a node found in this
		/// collection. This callback is invoked on each tag in the collection when the GQSerializer
		/// encounters a tag in the document that is also held in this collection. The tag start
		/// callback provides an enum of the type of tag about to be serialized. The user can return
		/// true or false to tell the serializer whether to proceed with serializing the tag, or to
		/// skip it entirely. Note that returning false (skip) will cause the node and its
		/// descendants to be omitted from serialization.
		/// <para>&#160;</para>
		/// Note that of course, if the user decides to skip serializing the node at this phase, no
		/// further callbacks for the node in question will be called.
		/// </summary>
		/// <param name="callback">
		/// The user defined callback for managing the start of serialization for a node that the
		/// user has added to this collection.
		/// </param>
		void SetOnTagStart(OnTagCallback callback);

		/// <summary>
		/// Sets the callback to be used during serialization of a node found in this collection.
		/// This callback is invoked for each attribute found in each tag in the collection when the
		/// GQSerializer encounters a tag in the document that is also held in this collection. The
		/// tag attribute callback provides an enum of the type of tag being serialized, a reference
		/// to the tag string that the user can populate in a custom fashion, as well as the current
		/// attribute and its value (if any) that is being processed. The attribute and its
		/// potentially empty value are provided as string_ref's, so that the user isn't required to
		/// take a copy of these items as string unless the user is sure they'd like them, at which
		/// time the boost::string_ref::to_string() member can be used.
		/// </summary>
		/// <param name="callback">
		/// The user defined callback for managing the serialization of attributes for a node that
		/// the user has added to this collection.
		/// </param>
		void SetOnTagAttribute(OnTagAttributeCallback callback);

		/// <summary>
		/// Sets the callback to be used during serialization of a the contents of a node found in
		/// this collection. This callback is invoked seeking tag body data from the user, but the
		/// user doesn't actually need to provide any custom body data. If this callback returns
		/// without the supplied string reference being populated, the existing/normal content of
		/// the tag will be serialized into the final output instead. Any nodes that are found in
		/// the content of course will also go through the user defined callbacks in this object.
		/// <para>&#160;</para>
		/// If user does populate the supplied string with some data, the user should return true
		/// when the user wishes the supplied string data to replace any immediate text within the
		/// tag alone, and should return false if the supplied string reference should entirely
		/// replace the contents of the node.
		/// <para>&#160;</para>
		/// To clarify, if the user pushed "Hey, this is some text!" to the supplied string
		/// reference, and returned true, then any existing node text will be omitted in the
		/// serialization, and replaced entirely with "Hey, this is some text!", while any non-text
		/// node children will be serialized and appended accordingly.
		/// <para>&#160;</para>
		/// If the supplied string is populated and the user returns false, then only the string
		/// data will be appended as-is to the tag contents. That is to say, all existing children
		/// of the tag (this includes text children) will be dropped, and the user supplied data
		/// will take its place, regardless if what the text represents.
		/// </summary>
		/// <param name="callback">
		/// The user defined callback for managing the start of serialization for a node that the
		/// user has added to this collection.
		/// </param>
		void SetOnTagContent(OnTagContentCallback callback);
		
	private:

		/// <summary>
		/// Checks if the following GumboNode* is a part of this collection.
		/// </summary>
		/// <param name="rawNode">
		/// The GumboNode* to search for.
		/// </param>
		/// <returns>
		/// True if the node is part of this collection, false otherwise.
		/// </returns>
		const bool Contains(const GumboNode* rawNode) const;

		/// <summary>
		/// Collection of GumboNode* objects extracted from the GQNode::m_node private member when
		/// GQNode objects are added to this collection.
		/// </summary>
		std::unordered_set<const GumboNode*> m_rawNodes;

		/// <summary>
		/// User defined callback for when serialization of a node found in this collection begins.
		/// </summary>
		OnTagCallback m_onTagStart;

		/// <summary>
		/// User defined callback for when an attribute found on a node found in this collection is
		/// being serialized.
		/// </summary>
		OnTagAttributeCallback m_onTagAttribute;

		/// <summary>
		/// User defined callback for when the content of a node found in this collection is being
		/// serialized.
		/// </summary>
		OnTagContentCallback m_onTagContent;

	};

} /* namespace gq */

