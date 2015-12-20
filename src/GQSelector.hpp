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

#pragma once

#include <memory>
#include <string>
#include <gumbo.h>
#include <vector>
#include <cassert>
#include <boost/utility/string_ref.hpp>

// For printing debug information about compiled selectors to the console.
#ifndef NDEBUG
#include <iostream>
#endif

namespace gq
{

	class GQNode;

	/// <summary>
	/// The GQSelector is the base class for all selectors in GQ. It handles simple and generic
	/// aspects of matching, such handling nth parameters in pseudo selectors and matching against
	/// specific html tags.
	/// </summary>
	class GQSelector
	{

		// So that the GQParser can copy/set the original source selector string at
		// the request of the user.
		friend class GQParser;

	public:

		enum class SelectorOperator
		{
			/// <summary>
			/// Dummy, matches no matter what.
			/// </summary>
			Dummy,
			
			/// <summary>
			/// Match against elements that contain no valid child elements.
			/// </summary>
			Empty,

			/// <summary>
			/// Match against elements that are the only child of their parent.
			/// </summary>
			OnlyChild,

			/// <summary>
			/// Match against elements that are the nth child of their parent.
			/// </summary>
			NthChild,

			/// <summary>
			/// Match against a specific type of html element.
			/// </summary>
			Tag
		};

		/// <summary>
		/// Not all selectors are simple. Some selectors search for children or even descendants of
		/// nodes. When such selectors are run, they are expected to return the most righthand
		/// object specified in the selector. As such, returning a simple boolean on the public
		/// interface is not sufficient to give users an accurate collection of matched nodes.
		/// 
		/// To meet this requirement, it's necessary to return a structure, rather than a simple
		/// bool yes or no that the supplied GQNode is a match, since while a match may have been
		/// found, the match may be in actuality a descendant of the supplied GQNode.
		/// </summary>
		struct GQMatchResult
		{

			// Only GQSelector and subclasses should be able to construct these.
			friend class GQSelector;
			friend class GQBinarySelector;
			friend class GQAttributeSelector;
			friend class GQUnarySelector;
			friend class GQTextSelector;

		public:

			/// <summary>
			/// Default destructor.
			/// </summary>
			~GQMatchResult();

			/// <summary>
			/// Gets the result of the match. May or may not be nullptr. Use the bool operator of
			/// this structure to conveniently determine if the match is nullptr or not.
			/// </summary>
			/// <returns>
			/// The node matched by the most right hand side of a single or combined selector. Will
			/// be nullptr if there was no successful match. Valid otherwise.
			/// </returns>
			const std::shared_ptr<GQNode> GetResult() const;			

			/// <summary>
			/// Determine if this result contains a valid match or not. 
			/// </summary>
			/// <returns>
			/// True if this object represents a successful match and contains a pointer to the node
			/// matching the most righthand side of the selector that matched, false otherwise.
			/// </returns>
			operator bool() const;

			const bool operator==(const bool other) const;

			const bool operator!=(const bool other) const;

		private:

			GQMatchResult(const GQNode* result = nullptr);

			GQNode* m_result;

		};

		GQSelector();

		/// <summary>
		/// Constructs a GQSelector with the specified operator. 
		/// </summary>
		/// <param name="op">
		/// The operator to define the functionality of this selector. 
		/// </param>
		GQSelector(SelectorOperator op);

		/// <summary>
		/// Construct a selector meant to match a single element. 
		/// </summary>
		/// <param name="matchType">
		/// Determine if the single element is bound to a specific type or not. 
		/// </param>
		GQSelector(const bool matchType);

		/// <summary>
		/// Construct a selector used to match nth-(child|last-child|of-type|last-of-type)
		/// selectors. The arguments, a and b, are the parsed result of the selector Nth parameter,
		/// a value specified like "nth-child(3n+3)". These values, combined with other parameters
		/// such as matchLast determine how to correctly select based on the supplied nth arguments.
		/// </summary>
		/// <param name="leftHandSideOfNth">
		/// The left-hand side of the nth argument. In the example nth-child(2n+3), this value would
		/// be 2 (two).
		/// </param>
		/// <param name="rightHandSideOfNth">
		/// The right-hand side of the nth argument. In the example nth-child(2n+3), this value would
		/// be 3 (three).
		/// </param>
		/// <param name="matchLast">
		/// Determine if the matching is from the last, rather than the first. This will modify how
		/// the supplied offsets calculate which element to select.
		/// </param>
		/// <param name="matchType">
		/// Determine if matching is bound to a specific type. 
		/// </param>
		GQSelector(const int leftHandSideOfNth, const int rightHandSideOfNth, const bool matchLast, const bool matchType);

		/// <summary>
		/// Constructs a GQSelector intended solely to match a specific element based on its tag
		/// type.
		/// </summary>
		/// <param name="tagTypeToMatch">
		/// The type of tag/element to match. 
		/// </param>
		GQSelector(GumboTag tagTypeToMatch);

		/// <summary>
		/// Default destructor.
		/// </summary>
		virtual ~GQSelector();

		/// <summary>
		/// Gets the GumboTag that this selector is to match against. If this selector is not built with the ::Tag matching operator, then the
		/// result will be empty. Default value is empty/nothing. 
		/// </summary>
		/// <returns>
		/// The GumboTag, if any, that this selector is to match against.
		/// </returns>
		const GumboTag GetTagTypeToMatch() const;	

		/// <summary>
		/// Gets the normalized tag name that this selector should match against. 
		/// </summary>
		/// <returns>
		/// The normalized tag name that this selector should match against. Empty no tag is
		/// specified.
		/// </returns>
		const boost::string_ref GetNormalizedTagTypeToMatch() const;
		
		/// <summary>
		/// Get a collection of attributes that this selector requires for the sake of matching.
		/// These attributes are used internally in GQNode and GQDocument to filter potential match
		/// candidates by.
		/// </summary>
		/// <returns>
		/// A collection of attributes that can be used to narrow down potential match candidates. 
		/// </returns>
		const std::vector< std::pair<boost::string_ref, boost::string_ref> >& GetMatchTraits() const;

		/// <summary>
		/// Check if this selector is a match against the supplied node. 
		/// </summary>
		/// <param name="node">
		/// The node to attempt to match against. 
		/// </param>
		/// <returns>
		/// True if this selector was successfully matched against the supplied node, false
		/// otherwise.
		/// </returns>
		virtual const GQMatchResult Match(const GQNode* node) const;

		/// <summary>
		/// Recursively tests for matches against the supplied node and all of its descendants,
		/// returning a collection of all nodes that were positively matched by this selector.
		/// </summary>
		/// <param name="node">
		/// The node to match against, as well as it's descendants. 
		/// </param>
		/// <returns>
		/// A collection of all nodes matched by this selector. 
		/// </returns>
		void MatchAll(const std::shared_ptr<GQNode>& node, std::vector< std::shared_ptr<GQNode> >& results) const;

		/// <summary>
		/// Accepts an existing collection of nodes and removes all nodes in the collection that do
		/// not positively match against this selector.
		/// </summary>
		/// <param name="nodes">
		/// A collection of nodes to filter. 
		/// </param>
		void Filter(std::vector< std::shared_ptr<GQNode> >& nodes) const;

		/// <summary>
		/// Fetches the original selector string that the selector was built from. This will only be
		/// set to a non-empty string if it was specified at selector construction that the original
		/// string should be copied and retained.
		/// </summary>
		/// <returns>
		/// </returns>
		boost::string_ref GetOriginalSelectorString() const;

	protected:

		/// <summary>
		/// Allows subclasses to modify the tag to match. Updates the member variables that are returned
		/// from ::GetTagTypeToMatch() and GetNormalizedTagTypeToMatch().
		/// </summary>
		/// <param name="tag">
		/// The tag type that this selector should match against, if any.
		/// </param>
		void SetTagTypeToMatch(const GumboTag tag);

		/// <summary>
		/// Adds the supplied trait to the selector's internal traits collection. These traits are
		/// used for quickly finding potential match canadidates as opposed to doing manual,
		/// recursive match testing.
		/// </summary>
		/// <param name="key">
		/// The trait key. The key should be the name of some attribute in a potential candidate. 
		/// </param>
		/// <param name="value">
		/// The trait value. The value should either be an exact value match sought in a potential
		/// candidate, or a "*" if specifying an exact value match is not possible. In the event of
		/// an asterik value, any node containing the supplied attribute will be selected as a
		/// potential match candidate, so use wisely.
		/// </param>
		void AddMatchTrait(boost::string_ref key, boost::string_ref value);

	private:
		
		/// <summary>
		/// If the selector is a tag selector, this member will store the type of GumboTag the
		/// selector must match against.
		/// </summary>
		GumboTag m_tagTypeToMatch = GumboTag(0);

		/// <summary>
		/// The value of gumbo_normalized_tagname(m_tagTypeToMatch), if m_tagTypeToMatch !=
		/// GUMBO_TAG_UNKNOWN.
		/// </summary>
		boost::string_ref m_normalizedTagTypeToMatch;

		/// <summary>
		/// Defines how the matching in this selector will work. Based on the option, the text to be
		/// matched will be matched in different ways. See comments in the operator class.
		/// </summary>
		SelectorOperator m_selectorOperator = SelectorOperator::Dummy;

		/// <summary>
		/// If this selector was constructed to do an nth-* match, this member will hold the value
		/// of the left hand side (left of the N) of the nth parameter. In the event that the parsed
		/// value of the nth parameter is something like "odd", "even", "-n+3", the values supplied
		/// in the constructor will already be accurately set accordingly by
		/// GQParser::ParseNth(...).
		/// </summary>
		int m_leftHandSideOfNth = 0;

		/// <summary>
		/// If this selector was constructed to do an nth-* match, this member will hold the value
		/// of the right hand side (right of the N) of the nth parameter. In the event that the
		/// parsed value of the nth parameter is something like "odd", "even", "-n+3", the values
		/// supplied in the constructor will already be accurately set accordingly by
		/// GQParser::ParseNth(...).
		/// </summary>
		int m_rightHandSideOfNth = 0;

		/// <summary>
		/// If this member is true, then in our matching code, it means we're doing some sort of 
		/// last match, where we're takes to match the last or nth last or last-of-type.
		/// </summary>
		bool m_matchLast;

		/// <summary>
		/// This member is difficult to name correctly, in a way that makes its purpose very clear.
		/// This member has absolutely nothing to do with matching a GumboTag. Rather, this bool is
		/// used for correctly counting sublings and children in nth-child/only-child type selectors
		/// where type is a factor. For example, nth-last-of-type and only-of-type would both
		/// require this member to be set to "true". The reason for this is what when we're
		/// iterating through siblings/children on such selectors, we need to ignore anything not of
		/// the same type as the node we're trying to match when counting. nth-last-of-type(2) is
		/// very easy to calculate when you only count siblings/children of the same type.
		/// <para>&#160;</para>
		/// I know I've explained this to death, but I want to save the user the burden I had of
		/// resolving the true meaning/purpose of this member.
		/// </summary>
		bool m_matchType;	

		/// <summary>
		/// A copy of the original selector string. Is only set if the user requested that the
		/// original string be retained when constructing the selector. Default constructed to empty
		/// string.
		/// </summary>
		std::string m_originalSelectorString;

		/// <summary>
		/// Stores "traits" about potential candidates. This is used heavily in the core matching
		/// API for quickly looking up candidates based on such traits. Manual recursive matching is
		/// highly undesirable because it absolutely murders performance. As such, it is absolutely
		/// required that any selector of any type generates and stores accurate traits for finding
		/// matches. Manual searching can/will happen but only when absolutely necessary. It is a
		/// goal for this project that manual searching should be eliminated as a requirement
		/// altogether.
		/// </summary>
		std::vector< std::pair<boost::string_ref, boost::string_ref> > m_matchTraits;

		/// <summary>
		/// Init member defaults across multiple constructors.
		/// </summary>
		void InitDefaults();

		/// <summary>
		/// For use by the public MatchAll function. Aids in recursively matching down a chain of
		/// descandants.
		/// </summary>
		/// <param name="node">
		/// The present node to process. 
		/// </param>
		/// <param name="nodes">
		/// The existing collection of matched nodes to append matches to. 
		/// </param>
		void MatchAllInto(const std::shared_ptr<GQNode>& node, std::vector< std::shared_ptr<GQNode> >& nodes) const;

	};

	typedef std::shared_ptr<GQSelector> SharedGQSelector;

} /* namespace gq */
