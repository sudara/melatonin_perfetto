/*
BEGIN_JUCE_MODULE_DECLARATION

 ID:                 melatonin_perfetto
 vendor:             Sudara
 version:            1.1.0
 name:               Melatonin Perfetto
 description:        Perfetto module for JUCE
 license:            MIT
 dependencies:       juce_core
 minimumCppStandard: 17

END_JUCE_MODULE_DECLARATION
*/

#pragma once

#ifndef PERFETTO
    #define PERFETTO 0
#endif

#if PERFETTO

    #include <chrono>
    #include <fstream>
    #include <juce_core/juce_core.h>
    #include <perfetto.h>
    #include <thread>

PERFETTO_DEFINE_CATEGORIES (
    perfetto::Category ("component")
        .SetDescription ("Component"),
    perfetto::Category ("dsp")
        .SetDescription ("dsp"));

class MelatoninPerfetto
{
public:
    MelatoninPerfetto (const MelatoninPerfetto&) = delete;

    static MelatoninPerfetto& get()
    {
        static MelatoninPerfetto instance;
        return instance;
    }

    void beginSession (uint32_t buffer_size_kb = 80000)
    {
        perfetto::TraceConfig cfg;
        cfg.add_buffers()->set_size_kb (buffer_size_kb); // 80MB is the default
        auto* ds_cfg = cfg.add_data_sources()->mutable_config();
        ds_cfg->set_name ("track_event");
        beginSession (cfg);
    }

    void beginSession (const perfetto::TraceConfig& cfg)
    {
        session = perfetto::Tracing::NewTrace();
        session->Setup (cfg);
        session->StartBlocking();
    }

    // Returns the file where the dump was written to (or a null file if an error occurred)
    // the return value can be ignored if you don't need this information
    juce::File endSession()
    {
        // Make sure the last event is closed for this example.
        perfetto::TrackEvent::Flush();

        // Stop tracing
        session->StopBlocking();

        return writeFile();
    }

    static juce::File getDumpFileDirectory()
    {
    #if JUCE_WINDOWS
        return juce::File::getSpecialLocation (juce::File::SpecialLocationType::userDesktopDirectory);
    #else
        return juce::File::getSpecialLocation (juce::File::SpecialLocationType::userHomeDirectory).getChildFile ("Downloads");
    #endif
    }

private:
    MelatoninPerfetto()
    {
        perfetto::TracingInitArgs args;
        // The backends determine where trace events are recorded. For this example we
        // are going to use the in-process tracing service, which only includes in-app
        // events.
        args.backends = perfetto::kInProcessBackend;
        perfetto::Tracing::Initialize (args);
        perfetto::TrackEvent::Register();
    }

    juce::File writeFile()
    {
        // Read trace data
        std::vector<char> trace_data (session->ReadTraceBlocking());

        const auto file = getDumpFileDirectory();

    #if JUCE_DEBUG
        auto mode = juce::String ("-DEBUG-");
    #else
        auto mode = juce::String ("-RELEASE-");
    #endif

        const auto currentTime = juce::Time::getCurrentTime().formatted ("%Y-%m-%d_%H%M");
        const auto childFile = file.getChildFile ("perfetto" + mode + currentTime + ".pftrace");

        if (auto output = childFile.createOutputStream())
        {
            output->setPosition (0);
            output->write (&trace_data[0], trace_data.size() * sizeof (char));

            DBG ("Wrote perfetto trace to: " + childFile.getFullPathName());

            return childFile;
        }

        DBG ("Failed to write perfetto trace file. Check for missing permissions.");
        jassertfalse;
        return juce::File {};
    }

    std::unique_ptr<perfetto::TracingSession> session;
};

/*

   There be dragons here. Serious C++ constexpr dragons.

   Deriving the function name produces an ugly string:
        auto AudioProcessor::processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &)::(anonymous class)::operator()()::(anonymous class)::operator()(uint32_t) const

   Without dragons, trying to trim the string results in it happening at runtime.

   With dragons, we get a nice compile time string:
        AudioProcessor::processBlock

*/

namespace melatonin
{
    // This wraps our compile time strings (coming from PRETTY_FUNCTION, etc) in lambdas
    // so that they can be used as template parameters
    // This lets us trim the strings while still using them at compile time
    // https://accu.org/journals/overload/30/172/wu/#_idTextAnchor004
    #define WRAP_COMPILE_TIME_STRING(x) [] { return (x); }
    #define UNWRAP_COMPILE_TIME_STRING(x) (x)()

    template <typename CompileTimeLambdaWrappedString>
    constexpr auto compileTimePrettierFunction (CompileTimeLambdaWrappedString wrappedSrc)
    {
    // if we're C++20 or higher, ensure we're compile-time
    #if __cplusplus >= 202002L
        // This should never assert, but if so,  report it on this issue:
        // https://github.com/sudara/melatonin_perfetto/issues/13#issue-1558171132
        if (!std::is_constant_evaluated())
            jassertfalse;
    #endif

        constexpr auto src = UNWRAP_COMPILE_TIME_STRING (wrappedSrc);
        constexpr auto size = std::string_view (src).size(); // -1 to ignore the /0
        std::array<char, size> result {};

        // loop through the source, building a new truncated array
        // see: https://stackoverflow.com/a/72627251
        for (size_t i = 0; i < size; ++i)
        {
            // wait until after the return type (first space in the string)
            if (src[i] == ' ')
            {
                ++i; // skip the space

                // MSVC has an additional identifier after the return type: __cdecl
                if (src[i + 1] == '_')
                    i += 8; // skip __cdecl and the space afterwards

                size_t j = 0;

                // build result, stop when we hit the arguments
                // clang and gcc use (, MSVC uses <
                while ((src[i] != '(' && src[i] != '<') && i < size && j < size)
                {
                    result[j] = src[i];
                    ++i; // increment character in source
                    ++j; // increment character in result
                }

                // really ugly clean up after msvc, remove the extra :: before <lambda_1>
                if (src[i] == '<')
                {
                    result[j-2] = '\0';
                }
                return result;
            }
        }
        return result;
    }

    namespace test
    {
        // a lil test helper so we don't go crazy
        template <size_t sizeResult, size_t sizeTest>
        constexpr bool strings_equal (const std::array<char, sizeResult>& result, const char (&test)[sizeTest])
        {
            // sanity check
            static_assert (sizeTest > 1);
            static_assert (sizeResult + 1 >= sizeTest); // +1 for the /0

            std::string_view resultView (result.data(), sizeTest);
            std::string_view testView (test, sizeTest);
            return testView == resultView;
        }

        // testing at compile time isn't fun (no debugging) so dumb things like this help:
        constexpr std::array<char, 10> main { "main" };
        static_assert (strings_equal (main, "main"));

        // ensure the return type is removed
        static_assert (strings_equal (compileTimePrettierFunction (WRAP_COMPILE_TIME_STRING ("int main")), "main"));

        // clang example
        static_assert (strings_equal (compileTimePrettierFunction (WRAP_COMPILE_TIME_STRING ("void AudioProcessor::processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &)::(anonymous class)::operator()()::(anonymous class)::operator()(uint32_t) const")), "AudioProcessor::processBlock"));

        // msvc example
        static_assert (strings_equal (compileTimePrettierFunction (WRAP_COMPILE_TIME_STRING ("void __cdecl AudioProcessor::processBlock::<lambda_1>::operator")), "AudioProcessor::processBlock"));
    }
}

    // Et voilà! Our nicer macros.
    // This took > 20 hours, hope the DX is worth it...
    // The separate constexpr calls are required for `compileTimePrettierFunction` to remain constexpr
    // in other words, they can't be inline with perfetto::StaticString, otherwise it will go runtime
    #define TRACE_DSP(...)                                                                                                            \
        constexpr auto pf = melatonin::compileTimePrettierFunction (WRAP_COMPILE_TIME_STRING (PERFETTO_DEBUG_FUNCTION_IDENTIFIER())); \
        TRACE_EVENT ("dsp", perfetto::StaticString (pf.data()), ##__VA_ARGS__)
    #define TRACE_COMPONENT(...)                                                                                                      \
        constexpr auto pf = melatonin::compileTimePrettierFunction (WRAP_COMPILE_TIME_STRING (PERFETTO_DEBUG_FUNCTION_IDENTIFIER())); \
        TRACE_EVENT ("component", perfetto::StaticString (pf.data()), ##__VA_ARGS__)

#else // if PERFETTO
    #define TRACE_EVENT_BEGIN(category, ...)
    #define TRACE_EVENT_END(category)
    #define TRACE_EVENT(category, ...)
    #define TRACE_DSP(...)
    #define TRACE_COMPONENT(...)
#endif
