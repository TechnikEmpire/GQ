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

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include "GQStrRefHash.hpp"

/*
	Special note for a special snowflake.

	The msvc project file for GQ disables error 4503 in the project properties. I disable
	this because I'm using psychotic templated objects that are rather complex and thus generate
	long decorated names that are all truncated by the compiler due to arbitrary dumb limits
	that shouldn't be around in 2015.

	If you're thinking ERMERGOSH HE DISABLED TEH WARNINGS:
	
		"The correctness of the program, however, is unaffected by the truncated name."

		Source: http://msdn.microsoft.com/en-us/library/074af4b6%28v=vs.80%29.aspx
*/

namespace gq
{

	class GQNode;	

	/// <summary>
	/// The GQTreeMap class serves the purpose of indexing an HTML document and all of its elements
	/// in a way that maximizes lookup speed for selectors. Without such a structure, every single
	/// selector run is doomed to traverse the entire document, node-by-node. The GQTreeMap object
	/// stores a list of all element from a certain scope of the document, index by attribute names
	/// and values.
	/// <para>&#160;</para>
	/// The GQTreeMap is created and managed exclusively within the GQDocument object, and is then
	/// supplied to all child nodes are they are created when the GQDocument object is created or
	/// parses some HTML.
	/// <para>&#160;</para>
	/// To further clarify "scope" simply corresponds to a specific html element within the
	/// document, all of which are identified by a specially generated unique value which can also
	/// be subtracted from to reveal the unique identifier of its parent. So when we search by
	/// "scope", we're searching at a specific node, down through all of its descendants. This
	/// allows searching within specific nodes or previous search results with selectors, without
	/// having to process any elements that preceed the object identified by the "scope" in the html
	/// document.
	/// <para>&#160;</para>
	/// Additionally, it's worth noting that this map treats normalized tag names as attributes as
	/// well, among other things. For more on that, look at the GQSpecialTraitKeys class.
	/// </summary>
	class GQTreeMap
	{	

		/// <summary>
		/// GQNode and its descendants are the only ones who actually need to access this object and
		/// its static members.
		/// </summary>
		friend class GQNode;
		friend class GQDocument;

	public:

		~GQTreeMap();

	private:
		
		typedef std::multimap<boost::string_ref, boost::string_ref> AttributeMap;

		GQTreeMap();		

		GQTreeMap(const GQTreeMap&) = delete;
		GQTreeMap& operator=(const GQTreeMap&) = delete;

		/// <summary>
		/// Adds an attribute map for the supplied node. The unique ID of the supplied node is
		/// extracted and then useds as the index by which the supplied attribute map will be stored
		/// and referenced.
		/// </summary>
		/// <param name="node">
		/// The node that the supplied attribute map belongs to. 
		/// </param>
		/// <param name="nodeAttributeMap">
		/// The pre-built attribute map which has mapped the properties (attributes) of this node
		/// for quick lookup and matching. Node that the tag of the node will also be built into
		/// this map, but that the key is randomly generated at runtime. To get the key that is used
		/// for storing normalized tag names, use the static member ::GetTagAttributeKey().
		/// </param>
		void AddNodeToMap(boost::string_ref scope, const GQNode* node, const AttributeMap& nodeAttributeMap);

		/// <summary>
		/// Gets a collection of nodes that have the supplied attribute with the provided scope.
		/// Node that normalized tag names also count as an attribute, but the attribute name needs
		/// to fetched from the ::GetTagKey() member and the attribute value must be a specific
		/// normalized tag name. Passing ::GetTagKey() to this overload is invalid, because the tag
		/// name needs to be specified as the attribute value, and this overload takes no value
		/// parameter. Use the overload that takes the exact value for such lookups.
		/// </summary>
		/// <param name="scope">
		/// The scope to search within. The scope is the value of GQNode::GetUniqueId(). 
		/// </param>
		/// <param name="attribute">
		/// The attribute which must exist. 
		/// </param>
		/// <returns>
		/// A collection of nodes which may contain zero or more elements, depending on how many
		/// elements matched the supplied parameters within the supplied scope.
		/// </returns>
		const std::vector< const GQNode* >* Get(boost::string_ref scope, boost::string_ref attribute) const;

		/// <summary>
		/// Gets a collection of nodes that have the supplied attribute with the exact value
		/// supplied with the provided scope. Node that normalized tag names also count as an
		/// attribute, but the attribute name needs to fetched from the ::GetTagKey() member and the
		/// attribute value must be a specific normalized tag name. Passing ::GetTagKey() to this
		/// overload is valid, but note that the tag name needs to be specified as the attribute
		/// value.
		/// </summary>
		/// <param name="scope">
		/// The scope to search within. The scope is the value of GQNode::GetUniqueId(). 
		/// </param>
		/// <param name="attribute">
		/// The attribute which must exist. 
		/// </param>
		/// <param name="attribute">
		/// The attribute value which must exactly match. 
		/// </param>
		/// <returns>
		/// A collection of nodes which may contain zero or more elements, depending on how many
		/// elements matched the supplied parameters within the supplied scope.
		/// </returns>
		const std::vector< const GQNode* >* Get(boost::string_ref scope, boost::string_ref attribute, boost::string_ref attributeValue) const;

		/// <summary>
		/// Empties the map.
		/// </summary>
		void Clear();

		/// <summary>
		/// For readability. This holds a collection of nodes with a certain attribute value. This
		/// object is the second element in the key/value pair for an unordered_map. The first
		/// element, the key, is an attribute name. This type of collection as the second parameter
		/// is necessary for multiple attribute values. Indexing by multiple values is required for
		/// attributes such as the "class" attribute, where the value can be a whitespace separated
		/// list of multiple distinct values. When indexing such attributes, we split the single
		/// whitespace separated list into multiple individual entries, pushing them to a map like
		/// this.
		/// </summary>
		typedef std::unordered_map<boost::string_ref, std::vector< const GQNode* >, StringRefHasher> ValueToNodesMap;

		/// <summary>
		/// Attribute maps take an attribute name as a key, and return a map which takes attribute
		/// values as as its key. Using both maps, if a match to an attribute with a specific value
		/// is found, the user gets a vector of all nodes which match the supplied attribute and
		/// value. If the attribute is found and the user simply wants to get nodes where the
		/// attribute exists, passing "*" as the key to the value map will yield all nodes in which
		/// the attribute exists.
		/// </summary>
		typedef std::unordered_map<boost::string_ref, ValueToNodesMap, StringRefHasher> CollectedAttributesMap;

		/// <summary>
		/// In order to enable quickly searching within a scope, we store shared pointers to built
		/// attribute maps within vectors and push those to a scope map. The key to this map is the
		/// GQNode::m_nodeUniqueIndexId private property. This property is generated when documents
		/// are parsed recursively, and gives an accurate, unique ID for each node.
		/// <para>&#160;</para>
		/// The GQNode::m_nodeUniqueIndexId property is built by concatenating its own index within
		/// parent as a string on to the same property of its parent. For example, given the tree
		/// A-A-C, C would have an m_nodeUniqueIndexId property of "002" while A-B-A would have an
		/// m_nodeUniqueIndexId property of "010". As such, dropping the index within parent off of
		/// the property yields the unique id of the parent.
		/// <para>&#160;</para>
		/// By using this index to store and organize attribute maps of nodes throughout the whole
		/// document, we can allow for quick jumping to potential matches and quick searching within
		/// a scope, such as selecting against a previously returned collection. If we want to
		/// search within a collection, we certainly don't want to bother looking for attributes
		/// that don't exist within the scope we're searching.
		/// </summary>
		typedef std::unordered_map<boost::string_ref, CollectedAttributesMap, StringRefHasher> ScopedAttributeMap;

		/// <summary>
		/// Scoped attributes which map nodes based on attributes existing and also their values.
		/// Normalized tag names are also considered attributes that are mapped here as well. The
		/// key for these is randomly decided at runtime to avoid having collisions with actual in
		/// the wild made up attributes, and also to avoid detection and deliberate fudging of the
		/// operation of this library.
		/// </summary>
		ScopedAttributeMap m_scopedAttributes;		

	};

	typedef std::unique_ptr<GQTreeMap> UniqueGQTreeMap;

} /* namespace gq */
