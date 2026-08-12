/*
	oscpack -- Open Sound Control (OSC) packet manipulation library
	http://www.rossbencina.com/code/oscpack

	OSC address pattern matching, per the OSC 1.0 specification.

	This file is an addition to the oscpack fork maintained by Limbic Media;
	upstream oscpack has never shipped address pattern matching. It is made
	available under the same terms as the rest of oscpack.
*/
#include "OscAddressPattern.h"

namespace osc{

namespace{

// Match a '[...]' character class beginning at p against the character c.
// On success p is advanced past the closing ']', so that a caller can probe
// with a throw-away pointer and still learn where the class ended.
bool MatchCharacterClass( const char *&p, const char *pEnd, char c )
{
	++p; // skip '['

	bool negated = false;
	if( p != pEnd && *p == '!' ){
		negated = true;
		++p;
	}

	bool matched = false;
	while( p != pEnd && *p != ']' ){
		// 'a-z' is a range, but a '-' immediately before the closing ']'
		// (or at the end of the pattern) is a literal '-'. The distances are
		// compared rather than forming 'p + 2', which would be a pointer more
		// than one past the end when p is the final character.
		if( (pEnd - p) > 2 && *(p + 1) == '-' && *(p + 2) != ']' ){
			if( *p <= c && c <= *(p + 2) )
				matched = true;
			p += 3;
		}else{
			if( *p == c )
				matched = true;
			++p;
		}
	}

	if( p == pEnd )
		return false; // an unterminated '[' never matches

	++p; // skip ']'

	return matched != negated;
}

// Match a '{a,b}' alternation beginning at p, together with everything that
// follows it in the pattern, against the remainder of the segment. The OSC 1.0
// grammar defines the alternation as a comma separated list of literal
// strings, so the alternatives are not themselves patterns and do not nest.
bool MatchBraceAlternation( const char *p, const char *pEnd,
		const char *s, const char *sEnd )
{
	const char *close = p + 1;
	while( close != pEnd && *close != '}' )
		++close;

	if( close == pEnd )
		return false; // an unterminated '{' never matches

	const char *alt = p + 1;
	for(;;){
		const char *altEnd = alt;
		while( altEnd != close && *altEnd != ',' )
			++altEnd;

		if( (sEnd - s) >= (altEnd - alt) ){
			bool prefixMatched = true;
			const char *a = alt;
			const char *b = s;
			while( a != altEnd ){
				if( *a != *b ){
					prefixMatched = false;
					break;
				}
				++a;
				++b;
			}

			// The tail of the pattern has to match what the alternative
			// leaves behind, otherwise this branch is not a real match.
			if( prefixMatched && MatchAddressPatternSegment(
					close + 1, pEnd, s + (altEnd - alt), sEnd ) )
				return true;
		}

		if( altEnd == close )
			return false;

		alt = altEnd + 1;
	}
}

} // namespace

bool MatchAddressPatternSegment( const char *patternBegin, const char *patternEnd,
		const char *segmentBegin, const char *segmentEnd )
{
	// Iterative backtracking for '*'; recursion only for '{}' alternation.
	const char *p = patternBegin;
	const char *s = segmentBegin;
	const char *starP = 0;
	const char *starS = 0;

	while( s != segmentEnd ){
		if( p != patternEnd ){
			if( *p == '*' ){
				// Record where to resume should the rest fail to match.
				starP = p++;
				starS = s;
				continue;
			}

			if( *p == '{' ){
				if( MatchBraceAlternation( p, patternEnd, s, segmentEnd ) )
					return true;
				// otherwise fall through to '*' backtracking below, so an
				// earlier '*' still gets the chance to consume more
			}else if( *p == '[' ){
				const char *probe = p;
				if( MatchCharacterClass( probe, patternEnd, *s ) ){
					p = probe;
					++s;
					continue;
				}
				// otherwise fall through to '*' backtracking below
			}else if( *p == '?' || *p == *s ){
				++p;
				++s;
				continue;
			}
		}

		if( starP ){
			p = starP + 1;
			s = ++starS;
			continue;
		}

		return false;
	}

	// The segment is exhausted, so whatever is left of the pattern has to
	// match the empty string.
	while( p != patternEnd && *p == '*' )
		++p;

	if( p != patternEnd && *p == '{' )
		return MatchBraceAlternation( p, patternEnd, s, segmentEnd );

	return p == patternEnd;
}

bool MatchAddressPattern( const char *pattern, const char *address )
{
	if( pattern == 0 || address == 0 )
		return false;

	// OSC addresses and patterns are absolute, so both have to begin with a
	// '/'. Checking here also rejects the empty string, which the loop below
	// would otherwise accept as matching another empty string.
	if( *pattern != '/' || *address != '/' )
		return false;

	const char *p = pattern;
	const char *a = address;

	while( *p != '\0' && *a != '\0' ){
		// Both sides have to be at a segment boundary.
		if( *p != '/' || *a != '/' )
			return false;
		++p;
		++a;

		const char *patternSegmentEnd = p;
		while( *patternSegmentEnd != '\0' && *patternSegmentEnd != '/' )
			++patternSegmentEnd;

		const char *addressSegmentEnd = a;
		while( *addressSegmentEnd != '\0' && *addressSegmentEnd != '/' )
			++addressSegmentEnd;

		if( !MatchAddressPatternSegment( p, patternSegmentEnd, a, addressSegmentEnd ) )
			return false;

		p = patternSegmentEnd;
		a = addressSegmentEnd;
	}

	// Both sides must be fully consumed: the segment counts have to agree.
	return *p == '\0' && *a == '\0';
}

} // namespace osc
