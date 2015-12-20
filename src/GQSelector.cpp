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

#include <unordered_set>
#include "GQSelector.hpp"
#include "GQNode.hpp"
#include "GQSpecialTraits.hpp"

namespace gq
{
	GQSelector::GQMatchResult::GQMatchResult(const GQNode* result) :m_result(const_cast<GQNode*>(result))
	{

	}

	GQSelector::GQMatchResult::~GQMatchResult()
	{

	}

	const std::shared_ptr<GQNode> GQSelector::GQMatchResult::GetResult() const
	{
		// Since it's not possible to construct a GQNode except through an interface which
		// guarantees that the node is created via make_shared, and GQDocument and its
		// GQTreeMap member holds references too all such constructed nodes, it should be
		// safe to hold a raw pointer here and call shared_from_this() if and when the user
		// actually wants a copy of the shared node.
		if (m_result == nullptr) { return nullptr; }
		return m_result->shared_from_this();
	}

	GQSelector::GQMatchResult::operator bool() const
	{
		return m_result != nullptr;
	}

	const bool GQSelector::GQMatchResult::operator==(const bool other) const
	{
		return (m_result != nullptr) == other;
	}

	const bool GQSelector::GQMatchResult::operator!=(const bool other) const
	{
		return (m_result != nullptr) != other;
	}

	GQSelector::GQSelector()
	{
		InitDefaults();
	}

	GQSelector::GQSelector(SelectorOperator op)
	{
		InitDefaults();
		m_selectorOperator = op;

		if (m_selectorOperator == SelectorOperator::Dummy)
		{
			// A dummy can match anything. So we add a trait that it can match against any tag type.
			AddMatchTrait(GQSpecialTraits::GetTagKey(), GQSpecialTraits::GetAnyValue());
		}

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_DEBUG_NFO
				std::cout
					<< u8"Built GQSelector with operator "
					<< static_cast<size_t>(op)
					<< u8"."
					<< std::endl;
			#endif
		#endif
	}

	GQSelector::GQSelector(const bool matchType)
	{
		InitDefaults();
		m_selectorOperator = SelectorOperator::OnlyChild;
		m_matchType = matchType;

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_DEBUG_NFO
				std::cout
					<< u8"Built GQSelector for only child matching"
					<< u8" with matching type set to "
					<< std::boolalpha << matchType
					<< u8"."
					<< std::endl;
			#endif
		#endif
	}

	GQSelector::GQSelector(const int leftHandSideOfNth, const int rightHandSideOfNth, const bool matchLast, const bool matchType)
	{
		InitDefaults();
		m_selectorOperator = SelectorOperator::NthChild;
		m_leftHandSideOfNth = leftHandSideOfNth;
		m_rightHandSideOfNth = rightHandSideOfNth;
		m_matchLast = matchLast;
		m_matchType = matchType;

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_DEBUG_NFO
			std::cout 
				<< u8"Built GQSelector for nth matching with lhs of "
				<< std::to_string(leftHandSideOfNth) 
				<< u8" and rhs of " 
				<< std::to_string(rightHandSideOfNth) 
				<< u8" match last set to "
				<< std::boolalpha << matchLast
				<< u8" and match type set to "
				<< std::boolalpha << matchType
				<< u8"."
				<< std::endl;
			#endif
		#endif
	}

	GQSelector::GQSelector(GumboTag tagTypeToMatch)
	{
		InitDefaults();
		m_selectorOperator = SelectorOperator::Tag;
		SetTagTypeToMatch(tagTypeToMatch);

		#ifndef NDEBUG
			#ifdef GQ_VERBOSE_DEBUG_NFO
			std::cout << "Built GQSelector for matching GumboTag " << gumbo_normalized_tagname(tagTypeToMatch) << u8"." << std::endl;
			#endif
		#endif
	}

	GQSelector::~GQSelector()
	{
	}

	const GumboTag GQSelector::GetTagTypeToMatch() const
	{
		return m_tagTypeToMatch;
	}

	const boost::string_ref GQSelector::GetNormalizedTagTypeToMatch() const
	{
		return m_normalizedTagTypeToMatch;
	}

	const std::vector< std::pair<boost::string_ref, boost::string_ref> >& GQSelector::GetMatchTraits() const
	{
		return m_matchTraits;
	}

	const GQSelector::GQMatchResult GQSelector::Match(const GQNode* node) const
	{

		switch (m_selectorOperator)
		{
			case SelectorOperator::Dummy:
			{				
				return GQMatchResult(node);
			}
			break;

			case SelectorOperator::Empty:
			{
				if (node->IsEmpty())
				{
					return GQMatchResult(node);
				}

				return false;
			}
			break;

			case SelectorOperator::OnlyChild:
			{
				const GQNode* parent = node->GetParent();
				if (parent == nullptr)
				{
					// Can't be a child without parents. :( Poor node. So sad.
					return false;
				}

				int count = 0;
				
				auto pNumChild = parent->GetNumChildren();
				
				for (size_t i = 0; i < pNumChild; i++)
				{
					auto child = parent->GetChildAt(i);

					if (m_matchType && node->GetTag() != child->GetTag())
					{
						// When m_matchType is true, we want to ignore all nodes that are not of the same type, because
						// in this circumstance, we'd be processing an only-of-type selector. So, to not screw up our
						// count, we ignore any non-element nodes and any nodes not of the same type as what we're trying
						// to match.
						continue;
					}

					count++;

					if (count > 1)
					{
						return false;
					}
				}

				if (count == 1)
				{
					return GQMatchResult(node);
				}

				return false;
			}
			break;

			case SelectorOperator::NthChild:
			{
				const GQNode* parent = node->GetParent();
				if (parent == nullptr)
				{
					// Can't be a child without parents. :( Poor node. So sad.
					return false;
				}

				// A valid child is a child that is either an element, or a is a node of exactly the same
				// type as the node we're trying to match. We need to skip everything else because in 
				// Gumbo Parser, we can have non-element nodes. However, in something like jquery using
				// selectors, these are not factored in, so to keep our counts equal to what you'd expect
				// in a real browser environment, we only count children as described.
				int validChildCount = 0;				

				// The actual index is "actual" in the sense that it is adjusted according to the process
				// described in the comments on validChildCount. This will almost certainly differ from
				// node->position_within_parent, which is why we keep this variable.
				int actualIndex = 0;

				// As we iterate up to our guaranteed match, we'll expand the nth formula and generate a 
				// collection of all node indices that the nth parameter (expanded) could possible generate
				// within the range of the parent child count. Then, later, we'll check to see if the final
				// matched "actualIndex" of our discovered node is found within these expanded values to
				// tell if we have a match or not.
				std::unordered_set<int> validNths;

				for (size_t j = 0; j < parent->GetNumChildren(); j++)
				{					
					auto child = parent->GetChildAt(j);
					if ((m_matchType && node->GetTag() != child->GetTag()))
					{
						// If m_matchType is true, we're not counting any children that are not the same
						// tag type as valid children. We're pretending that they don't exist. We're doing
						// this for situations last-of-type and nth-last-of-type. Whenever either of those
						// selectors are used, we pretend the only elements that exist are of the type
						// we're looking for, to make counting simple.
						continue;
					}

					// Expand the nth forumla and get the generated nth index based on the present
					// plain index. By doing this, we can store a list of all possible indices within
					// the scope of this node's parent that would classify as valid nth indices, and
					// thus a final "actualIndex" matching any of these indices is a match to the 
					// selector. I'm explaining the hell out of this because the original code was
					// as much of a copy and paste from sizzler as is possible and of course, it didn't
					// work right, so I had to re-figure this nth thing out from scratch.
					// XXX TODO - Write more nth selector tests to verify that this code holds.
					validNths.insert(((m_leftHandSideOfNth * validChildCount) + m_rightHandSideOfNth));

					// Once the child is found, we store its "true" index, aka the index after we've
					// ignored everything we don't want to count as real children for the sake of maths.
					if (child.get() == node)
					{						
						actualIndex = validChildCount;

						if (!m_matchLast)
						{
							// If we're not matching last (nth-last, last-of), we can just break here and
							// check the nodes index for a match. If not, we need to carry on counting
							// the total number of valid children (children we're not ignoring) so we
							// can properly convert the index to offset from the the "last" aka end.
							++validChildCount;
							break;
						}
					}	

					++validChildCount;
				}				

				if (m_matchLast)
				{
					// If we're matching from "last" we're matching count from the end aka total valid child count.
					// A valid child depends on if the child is an html element, and when we're matching types 
					// for selectors like (nth-last-of-type), then valid siblings are further restricted to sibling
					// with the same tag name.
					// 
					// Got a bit off topic, but when we're matching from the end, we want to convert the index from
					// a "count from start" to "count from end", so substracting count from start from total count 
					// gives us this. In the case that we're not matching last, we need to append 1, since these
					// type of pseudo selectors seem not to use zero based indices.
					actualIndex = validChildCount - actualIndex;
				}
				else 
				{
					// Increase the index to convert from zero based indices.
					actualIndex += 1;
				}				

				// Expand the nth calculation against the actual found index of the node. No matter what the 
				// composition of the nth parameter is, this will generate a proper index. We then are going
				// to either exactly match this, or exactly match this to all other valid generated nth values
				// for the selector, which we created while doing the loop to find the child in the first
				// place using this exact same formula.
				int nthIndex = ((m_leftHandSideOfNth * actualIndex) + m_rightHandSideOfNth);

				// Either the calculated nth index will be an exact match, or during the iterations up till
				// the found node, we will have generated a collection of valid nths for the specified selector
				// and we'll find our index in there. If neither of those are true, then we didn't match at all.
				if (nthIndex == actualIndex || (validNths.find(actualIndex) != validNths.end()))
				{
					return GQMatchResult(node);
				}

				return false;
			}
			break;

			case SelectorOperator::Tag:
			{				
				if (node->GetTag() == m_tagTypeToMatch)
				{
					return GQMatchResult(node);
				}

				return false;
			}
			break;		
		}

		return false;
	}

	void GQSelector::MatchAll(const std::shared_ptr<GQNode>& node, std::vector< std::shared_ptr<GQNode> >& results) const
	{
		#ifndef NDEBUG
		assert(node != nullptr && u8"In GQSelector::MatchAll(const GumboNode*, std::vector< std::shared_ptr<GQNode> >&) - Nullptr node supplied for matching.");
		#else
		if (node == nullptr) { throw std::runtime_error(u8"In GQSelector::MatchAll(const GumboNode*, std::vector< std::shared_ptr<GQNode> >&) - Nullptr node supplied for matching."); }
		#endif

		MatchAllInto(node, results);
	}

	void GQSelector::Filter(std::vector< std::shared_ptr<GQNode> >& nodes) const
	{
		nodes.erase(std::remove_if(nodes.begin(), nodes.end(), 
			[this](const std::shared_ptr<GQNode>& sharedNode)
			{
				return !Match(sharedNode.get());
			}
		), nodes.end());
	}

	boost::string_ref GQSelector::GetOriginalSelectorString() const
	{
		return boost::string_ref(m_originalSelectorString);
	}

	void GQSelector::SetTagTypeToMatch(const GumboTag tag)
	{
		m_tagTypeToMatch = tag;

		if (m_tagTypeToMatch != GUMBO_TAG_UNKNOWN)
		{
			const char* normalName = gumbo_normalized_tagname(m_tagTypeToMatch);
			m_normalizedTagTypeToMatch = boost::string_ref(normalName);

			// Add the tag type as a match trait
			AddMatchTrait(GQSpecialTraits::GetTagKey(), m_normalizedTagTypeToMatch);
		}
	}

	void GQSelector::AddMatchTrait(boost::string_ref key, boost::string_ref value)
	{
		auto pair = std::make_pair(key, value);
		if (std::find(m_matchTraits.begin(), m_matchTraits.end(), pair) == m_matchTraits.end())
		{
			m_matchTraits.emplace_back(std::move(pair));
		}
	}

	void GQSelector::InitDefaults()
	{
		m_matchType = false;
		m_leftHandSideOfNth = 0;
		m_rightHandSideOfNth = 0;
		m_matchLast = false;
		m_tagTypeToMatch = GUMBO_TAG_UNKNOWN;
	}

	void GQSelector::MatchAllInto(const std::shared_ptr<GQNode>& node, std::vector< std::shared_ptr<GQNode> >& nodes) const
	{
		#ifndef NDEBUG
		assert(node != nullptr && u8"In GQSelector::MatchAllInto(const GumboNode*, std::vector<const GumboNode*>&) - Nullptr node supplied for matching.");
		#else
		if (node == nullptr) { throw std::runtime_error(u8"In GQSelector::MatchAllInto(const GumboNode*, std::vector<const GumboNode*>&) - Nullptr node supplied for matching."); }
		#endif

		if (Match(node.get()))
		{
			nodes.push_back(node);
		}

		auto nNumChildren = node->GetNumChildren();
		
		for (size_t i = 0; i < nNumChildren; i++)
		{
			auto child = node->GetChildAt(i);
			MatchAllInto(child, nodes);
		}
	}	

} /* namespace gq */

