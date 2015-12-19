/*
* Adapted from https://github.com/google/gumbo-parser/blob/master/examples/serialize.cc
*
* Copyright (c) 2015 Kevin B. Hendricks, Stratford, Ontario,  All Rights Reserved.
* Copyright (c) 2015 Jesse Nicholson
* loosely based on a greatly simplified version of BeautifulSoup4 decode() routine
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once

#include <boost/utility/string_ref.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/functional/hash.hpp>
#include <unordered_set>
#include <gumbo.h>

namespace gq
{

	class GQNode;

	/// <summary>
	/// The GQSerializer can take a single Gumbo Parser node and convert its contents back into an
	/// HTML string. Two methods are exposed, one will serialize the node and its contents, the
	/// other will serialze only the contents of the node. This can be used behind the scenes on
	/// jquery like methods such as .Html() and .InnerHtml().
	/// </summary>
	class GQSerializer
	{

	public:

		/// <summary>
		/// Converts the supplied node and all of its contents back into an HTML string. This can be
		/// used behind the scenes to give the behavior of the jquery method .Html().
		/// </summary>
		/// <param name="node">
		/// The node to serialize along with all of its contents. 
		/// </param>
		/// <returns>
		/// The HTML string built from the node and all of its contents. 
		/// </returns>
		static std::string Serialize(const GQNode* node);

		/// <summary>
		/// Converts the supplied node and all of its contents back into an HTML string. This can be
		/// used behind the scenes to give the behavior of the jquery method .Html().
		/// </summary>
		/// <param name="node">
		/// The node to serialize along with all of its contents. 
		/// </param>
		/// <returns>
		/// The HTML string built from the node and all of its contents. 
		/// </returns>
		static std::string Serialize(const GumboNode* node);

		/// <summary>
		/// Converts the supplied node contents back into an HTML string, without including the HTML
		/// of the supplied node. Only its contents are serialized. This can be used behind the
		/// scenes to give the behavior of the jquery method .InnerHtml().
		/// </summary>
		/// <param name="node">
		/// The node containing the contents to serialize.
		/// </param>
		/// <returns>
		/// The HTML string built from the node contents.
		/// </returns>
		static std::string SerializeContent(const GQNode* node);

		/// <summary>
		/// Converts the supplied node contents back into an HTML string, without including the HTML
		/// of the supplied node. Only its contents are serialized. This can be used behind the
		/// scenes to give the behavior of the jquery method .InnerHtml().
		/// </summary>
		/// <param name="node">
		/// The node containing the contents to serialize.
		/// </param>
		/// <returns>
		/// The HTML string built from the node contents.
		/// </returns>
		static std::string SerializeContent(const GumboNode* node);

	private:

		GQSerializer();
		~GQSerializer();

		/// <summary>
		/// For using boost::string_ref in std/boost::unordered_map
		/// </summary>
		struct StringRefHasher
		{
			size_t operator()(const boost::string_ref& strRef) const
			{
				return boost::hash_range(strRef.begin(), strRef.end());
			}
		};

		/// <summary>
		/// List of tags that do not require a named closing tag.
		/// </summary>
		static const std::unordered_set<boost::string_ref, StringRefHasher> EmptyTags;

		/// <summary>
		/// Tags that should have newlines appended to.
		/// </summary>
		static const std::unordered_set<boost::string_ref, StringRefHasher> SpecialHandling;

		/// <summary>
		/// Gets a string representation of the tag name for the supplied node. 
		/// </summary>
		/// <param name="node">
		/// The node from which to extract the string tag name. 
		/// </param>
		/// <returns>
		/// A string representation of the tag name for the supplied node. 
		/// </returns>
		static std::string GetTagName(const GumboNode* node);

		/// <summary>
		/// Builds a correct string DOCTYPE declaration for the supplied node. 
		/// </summary>
		/// <param name="node">
		/// The document node from which to build the DOCTYPE declaration string. 
		/// </param>
		/// <returns>
		/// A string DOCTYPE declaration for the supplied document node. 
		/// </returns>
		static std::string BuildDocType(const GumboNode* node);

		/// <summary>
		/// Builds a string representation of the attributes supplied. 
		/// </summary>
		/// <param name="at">
		/// The attributes to serialize. 
		/// </param>
		/// <param name="noEntities">
		/// A bool used to indicate if special XML characters within any of the attribute values
		/// should be converted to character references or not.
		/// </param>
		/// <returns>
		/// A string containing all of the attribute names and values. 
		/// </returns>
		static std::string BuildAttributes(const GumboAttribute* at);//, const bool noEntities);
	};

} /* namespace gq */
