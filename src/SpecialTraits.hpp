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

#include <boost/utility/string_ref.hpp>

namespace gq
{

	/// <summary>
	/// The purpose of the GQSpecialTraits class is to define and contain special attribute names
	/// that can be used for indexing elements based on certain qualities which are not defined as
	/// standard html element attributes.
	/// <para>&#160;</para>
	/// For example, it may be desired to express in the document index that a certain element has
	/// children or descendants of a certain type, so that selectors with such expressions can
	/// quickly access potential match candidates and potential match candidates alone.
	/// <para>&#160;</para>
	/// However, because we also index elements/nodes by standard, defined html element attributes,
	/// it's necessary to avoid potential conflicts of shadowing/overriding real/standard attributes
	/// with made up attribute names. It's also necessary to avoid using static made up names for
	/// such invented keys, to avoid exploitation of this engine and its mechanics.
	/// <para>&#160;</para>
	/// To solve all of these problems, this class exists holding single-use/one-time generated
	/// random tags with static meanings via static members. To clarify that, take for example the
	/// need to index by normalized tag name. This class would define a static function like
	/// "GetTagKey()" which would provide a string used for indexing a "div". The API would never
	/// change, but at runtime, the actual value returned from this member would be randomly
	/// generated once per program lifecycle.
	/// <para>&#160;</para>
	/// As such, classes such as GQNode, GQDocument and GQSelector are safe to always index and
	/// search the same types of attributes by these keys and it's as close to a guarantee as
	/// possible that these names will be unique and thus not conflict with any natural or "in the
	/// wild" input.
	/// <para>&#160;</para>
	/// Additionally, where it's necessary to do so, this class may provide additional strings that
	/// are meant to be useds as values as well under special conditions. For example, to index a
	/// node as the last of its type, the key GetPseudoKey() would be required, and
	/// GetLastOfTypeValue() would be required as well.
	/// <para>&#160;</para>
	/// Note that this class is incomplete. Consider all of its structures/functionality completely
	/// up in the air, except for ::GetTagKey() and associated members. This class will grow and
	/// change over time, as I seek new ways to make indexing more and more specific to refine
	/// potential matches down more and more. At the time of this writing, matching isn't much
	/// faster than simply doing no work at all. The only real performance increases that can be
	/// done are by improving indexing to reduce returned candidate set sizes.
	/// </summary>
	class GQSpecialTraits
	{
	
	public:

		/// <summary>
		/// Gets the unique key used for indexing based on normalized tag name. 
		/// </summary>
		/// <returns>
		/// The unique key used for indexing based on normalized tag name. 
		/// </returns>
		static const boost::string_ref GetTagKey();

		/// <summary>
		/// Gets the unique key used for indexing based on various pseudo class attributes. For
		/// example, to index a node as the last of its type, GetPseudoKey() and
		/// GetLastOfTypeValue() would be required and would need to be combined to index such a
		/// node.
		/// </summary>
		/// <returns>
		/// The unique key used for indexing based on various pseudo class attributes.
		/// </returns>
		static const boost::string_ref GetPseudoKey();

		/// <summary>
		/// Gets the value used for indexing a node as having a specified attribute key, but without
		/// explicitly declaring the value of the attribute and thus requiring an exact match when
		/// doing lookups through the document map.
		/// </summary>
		/// <returns>
		/// The value used for indexing a node as having a specified attribute key.
		/// </returns>
		static const boost::string_ref GetAnyValue();

		/// <summary>
		/// Gets the value used for indexing a node as being the last child within its parent. As
		/// discussed elsewhere in documentation, GQ doesn't consider every GumbNode* to be a valid
		/// "child" in the same sense the Gumbo Parser does. Only nodes that are of type
		/// GUMBO_NODE_ELEMENT are consumed and managed as GQNodes. Just as another reminder, last
		/// child in the sense of this value is meant to describe the last GUMBO_NODE_ELEMENT within
		/// a parent.
		/// </summary>
		/// <returns>
		/// The value used for indexing a node as being the last child within its parent. 
		/// </returns>
		static const boost::string_ref GetLastChildValue();

		/// <summary>
		/// Gets the value used for indexing a node as being the last child of a certain type within
		/// its parent. As discussed elsewhere in documentation, GQ doesn't consider every GumbNode*
		/// to be a valid "child" in the same sense the Gumbo Parser does. Only nodes that are of
		/// type GUMBO_NODE_ELEMENT are consumed and managed as GQNodes. Just as another reminder,
		/// last child in the sense of this value is meant to describe the last GUMBO_NODE_ELEMENT
		/// within a parent.
		/// </summary>
		/// <returns>
		/// The value used for indexing a node as being the last child of a certain type within its
		/// parent.
		/// </returns>
		static const boost::string_ref GetLastChildOfTypeValue();

	private:

		GQSpecialTraits();
		~GQSpecialTraits();

		class RandomKey
		{

		public:

			RandomKey();

			const boost::string_ref Get() const;

		private:

			std::string m_str;

			static const std::string Chars;
		};

		static const RandomKey TagKey;

		static const RandomKey PseudoKey;
	};

} /* namespace gq */