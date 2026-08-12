/*
	oscpack -- Open Sound Control (OSC) packet manipulation library
	http://www.rossbencina.com/code/oscpack

	Example of OSC address pattern matching using osc::MatchAddressPattern().

	This file is an addition to the oscpack fork maintained by Limbic Media;
	upstream oscpack has never shipped address pattern matching. It is made
	available under the same terms as the rest of oscpack.
*/

#include <iostream>
#include <iomanip>
#include <cstddef>

#include "osc/OscAddressPattern.h"


// A handful of (pattern, address) pairs chosen to walk through the grammar
// supported by MatchAddressPattern(): literal segments, '*', '?', '[...]'
// character classes (including negation and ranges), and '{...}'
// alternation. Read top to bottom together with the printed result.
struct GrammarExample{
	const char *pattern;
	const char *address;
	const char *note;
};

static const GrammarExample kGrammarExamples[] = {
	{ "/lights/1/color", "/lights/1/color", "literal match" },
	{ "/lights/1/color", "/lights/2/color", "literal mismatch" },
	{ "/lights/*/color", "/lights/7/color", "'*' matches within a segment" },
	{ "/lights/*", "/lights/7/color", "'*' does NOT cross '/'" },
	{ "/ch?nnel/1", "/channel/1", "'?' matches exactly one character" },
	{ "/lights/[1-4]/color", "/lights/3/color", "'[1-4]' character range" },
	{ "/lights/[1-4]/color", "/lights/9/color", "range excludes '9'" },
	{ "/lights/[!0-4]/color", "/lights/9/color", "'[!0-4]' negated range" },
	{ "/scene/{sunrise,sunset}", "/scene/sunset", "'{a,b}' alternation" }
};

static const std::size_t kGrammarExampleCount =
		sizeof(kGrammarExamples) / sizeof(kGrammarExamples[0]);


// The practical use of address pattern matching: an OSC server binds
// handlers ("methods") to a fixed set of concrete addresses, but an
// incoming message's OSC Address Pattern is allowed to contain the special
// characters above, so a single incoming message can target several
// methods at once. Dispatch works by testing the incoming pattern against
// every registered address and invoking whichever ones match.
static const char *kRegisteredAddresses[] = {
	"/lights/1/color",
	"/lights/2/color",
	"/lights/3/color",
	"/lights/4/color",
	"/lights/1/brightness",
	"/mixer/gain"
};

static const std::size_t kRegisteredAddressCount =
		sizeof(kRegisteredAddresses) / sizeof(kRegisteredAddresses[0]);


int main(int argc, char* argv[])
{
	(void) argc; // suppress unused parameter warnings
	(void) argv; // suppress unused parameter warnings

	std::cout << "OSC address pattern grammar\n";
	std::cout << "----------------------------------------\n";

	for( std::size_t i = 0; i < kGrammarExampleCount; ++i ){
		const GrammarExample &example = kGrammarExamples[i];
		bool matched = osc::MatchAddressPattern( example.pattern, example.address );

		std::cout << std::left << std::setw(26) << example.pattern
			<< std::setw(20) << example.address
			<< (matched ? "match   " : "no match")
			<< "  (" << example.note << ")\n";
	}

	std::cout << "\n";
	std::cout << "Dispatching a message\n";
	std::cout << "----------------------------------------\n";

	const char *incomingPattern = "/lights/[1-3]/color";
	std::cout << "incoming address pattern: " << incomingPattern << "\n";
	std::cout << "registered addresses:\n";

	for( std::size_t i = 0; i < kRegisteredAddressCount; ++i ){
		const char *registeredAddress = kRegisteredAddresses[i];
		bool matched = osc::MatchAddressPattern( incomingPattern, registeredAddress );

		std::cout << "  " << std::left << std::setw(24) << registeredAddress
			<< (matched ? "-> dispatched\n" : "-> skipped\n");
	}

	return 0;
}
