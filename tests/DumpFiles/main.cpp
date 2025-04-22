#include <melatonin_perfetto/melatonin_perfetto.h>
#include <juce_core/juce_core.h>
#include <memory>
#include <vector>
#include <cstdlib>

#if ! PERFETTO
	#error PERFETTO must be 1 for this test!
#endif

int szudzikPair (int a, int b)
{
	TRACE_DSP();

	const auto A = a >= 0 ? 2 * a : -2 * a - 1;
	const auto B = b >= 0 ? 2 * b : -2 * b - 1;

	if (A >= B)
		return A * A + A + B;

	return A + B * B;
}

int main (int, char**)
{
	MelatoninPerfetto tracingSession;

	auto& rand = juce::Random::getSystemRandom();

	// we want to make sure these function calls don't get optimized away
	std::vector<int> values;

	for (auto i = 0; i < 100; ++i)
		values.push_back (szudzikPair (rand.nextInt(), rand.nextInt()));

	const auto dumpFile = tracingSession.endSession();

	if (dumpFile.existsAsFile())
		return EXIT_SUCCESS;

	return EXIT_FAILURE;
}
