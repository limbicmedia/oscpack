/*
	oscpack -- Open Sound Control (OSC) packet manipulation library
	http://www.rossbencina.com/code/oscpack

	OSC address pattern matching, per the OSC 1.0 specification.

	This file is an addition to the oscpack fork maintained by Limbic Media;
	upstream oscpack has never shipped address pattern matching. It is made
	available under the same terms as the rest of oscpack.
*/
#ifndef INCLUDED_OSCPACK_OSCADDRESSPATTERN_H
#define INCLUDED_OSCPACK_OSCADDRESSPATTERN_H

namespace osc{

// Match an OSC address pattern against a literal OSC address.
// Both are '/'-separated. Wildcards are scoped to a single segment:
// '*' and '?' never match '/'.
// Supports: '*', '?', '[abc]', '[a-z]', '[!a-z]', '{foo,bar}'.
bool MatchAddressPattern( const char *pattern, const char *address );

// Match a single segment of a pattern against a single segment of an address.
// Neither range may contain '/'. Exposed for callers that walk segments
// themselves.
bool MatchAddressPatternSegment( const char *patternBegin, const char *patternEnd,
		const char *segmentBegin, const char *segmentEnd );

} // namespace osc

#endif /* INCLUDED_OSCPACK_OSCADDRESSPATTERN_H */
