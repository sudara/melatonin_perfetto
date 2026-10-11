/*
BEGIN_JUCE_MODULE_DECLARATION

 ID:                 melatonin_perfetto
 vendor:             Sudara
 version:            1.4.0
 name:               Melatonin Perfetto
 description:        Perfetto module for JUCE
 license:            MIT
 dependencies:       juce_core
 minimumCppStandard: 17

END_JUCE_MODULE_DECLARATION
*/

#pragma once

// I'm lazy and just toggle perfetto right here
// But you can define it in your build system or in the file that includes this header
#ifndef PERFETTO
    #define PERFETTO 0
#endif

#if PERFETTO

    // allow granular toggles to be defined before this file
    #ifndef PERFETTO_ENABLE_TRACE_DSP
        #define PERFETTO_ENABLE_TRACE_DSP 1
    #endif

    #ifndef PERFETTO_ENABLE_TRACE_COMPONENT
        #define PERFETTO_ENABLE_TRACE_COMPONENT 1
    #endif

    // When on, TRACE_DSP_PUBLIC ships its chosen public label instead of the real function name,
    // so shipped binaries don't leak DSP architecture. Readable builds keep the real name.
    #ifndef PERFETTO_OBFUSCATE_DSP
        #define PERFETTO_OBFUSCATE_DSP 0
    #endif

    #include <chrono>
    #include <fstream>
    #include <juce_core/juce_core.h>
    #include <perfetto.h>

PERFETTO_DEFINE_CATEGORIES (
    perfetto::Category ("component")
        .SetDescription ("Component"),
    perfetto::Category ("dsp")
        .SetDescription ("dsp"));

namespace melatonin
{
    static constexpr std::uint64_t melatoninGlobalEventTrackId = 0xE1A70515;
}

class MelatoninPerfetto
{
public:
    explicit MelatoninPerfetto (const bool startTrace = true, juce::String filePrefix = "perfetto", juce::String fileSuffix = ".pftrace")
        : startTraceAutomatically (startTrace),
          prefix (filePrefix),
          suffix (fileSuffix)
    {
        perfetto::TracingInitArgs args;
        // The backends determine where trace events are recorded. For this example we
        // are going to use the in-process tracing service, which only includes in-app events.
        args.backends = perfetto::kInProcessBackend;

        // Traces are first written to a smaller, intermediate buffer, which defaults to 256KB. It's easy to overflow the buffer
        // if you have many traces per audio block. This sets the size of that buffer to a more comfortable 16MB.
        args.shmem_size_hint_kb = 16 * 1024;
        perfetto::Tracing::Initialize (args);
        perfetto::TrackEvent::Register();
        registerGlobalEventTrack();

        if (startTraceAutomatically)
            beginSession();
    }

    ~MelatoninPerfetto()
    {
        if (started)
            endSession();
    }

    void beginSession (const uint32_t buffer_size_kb = 80000)
    {
        perfetto::TraceConfig cfg;
        cfg.add_buffers()->set_size_kb (buffer_size_kb); // 80MB is the default

        traceFile = getNewFile();
        cfg.set_write_into_file (true);
        cfg.set_file_write_period_ms (1000);
        cfg.set_flush_period_ms (5000);
        cfg.set_output_path (traceFile.getFullPathName().toStdString());

        auto* ds_cfg = cfg.add_data_sources()->mutable_config();
        ds_cfg->set_name ("track_event");
        session = perfetto::Tracing::NewTrace();
        session->Setup (cfg);
        session->StartBlocking();
        started = true;
    }

    // Returns the file where the dump was written to (or a null file if an error occurred)
    // the return value can be ignored if you don't need this information
    juce::File endSession()
    {
        session->FlushBlocking();
        session->StopBlocking();
        started = false;
        return traceFile;
    }

    static juce::File getDumpFileDirectory()
    {
    #if JUCE_WINDOWS
        return juce::File::getSpecialLocation (juce::File::SpecialLocationType::userDesktopDirectory);
    #elif JUCE_MAC
        return juce::File::getSpecialLocation (juce::File::SpecialLocationType::userHomeDirectory).getChildFile ("Downloads");
    #elif JUCE_IOS
        // the Files app shows Documents when UIFileSharingEnabled is set
        return juce::File::getSpecialLocation (juce::File::SpecialLocationType::userDocumentsDirectory);
    #else
        // Linux: use temp directory for CI compatibility (no desktop/Downloads on headless systems)
        return juce::File::getSpecialLocation (juce::File::SpecialLocationType::tempDirectory);
    #endif
    }

private:
    bool startTraceAutomatically;
    bool started = false;
    juce::String prefix;
    juce::String suffix;
    std::unique_ptr<perfetto::TracingSession> session;
    juce::File traceFile;

    static void registerGlobalEventTrack()
    {
        auto track = perfetto::Track (melatonin::melatoninGlobalEventTrackId);
        auto desc = track.Serialize();
        desc.set_name ("Global Events");
        perfetto::TrackEvent::SetTrackDescriptor (track, desc);
    }

    [[nodiscard]] juce::File getNewFile() const
    {
        const auto directory = getDumpFileDirectory();

    #if JUCE_DEBUG
        auto mode = juce::String ("-DEBUG-");
    #else
        auto mode = juce::String ("-RELEASE-");
    #endif

        const auto currentTime = juce::Time::getCurrentTime().formatted ("%Y-%m-%d_%H%M");
        const auto childFile = directory.getNonexistentChildFile (prefix + mode + currentTime, suffix);
        return childFile;
    }
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
        // This should never assert, but if so, report it on this issue:
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
                    result[j - 2] = '\0';
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

    /**
      * Tracks an instant event, which shows up as an arrow on a separate "Global Events" track in the Perfetto UI.
      * @param cat The name of the category.
      * @param desc Describe the event - this is what shows up in the trace.
      */
    #define TRACE_GLOBAL_INSTANT(cat, desc) TRACE_EVENT_INSTANT (cat, desc, perfetto::Track (melatonin::melatoninGlobalEventTrackId))

#else
// allow people to keep perfetto helpers in-place even when disabled
    #define TRACE_EVENT_BEGIN(category, ...)
    #define TRACE_EVENT_END(category)
    #define TRACE_EVENT(category, ...)
    #define TRACE_GLOBAL_INSTANT(category, ...)
#endif

// Et voilà! Our nicer macros.
// This took > 20 hours, hope the DX is worth it...
// The separate constexpr calls are required for `compileTimePrettierFunction` to remain constexpr
// in other words, they can't be inline with perfetto::StaticString, otherwise it will go runtime

// we also can toggle dsp/component on/off individually to help clean up traces
#if PERFETTO_ENABLE_TRACE_DSP
    #define TRACE_DSP(...)                                                                                                                   \
        static constexpr auto pf = melatonin::compileTimePrettierFunction (WRAP_COMPILE_TIME_STRING (PERFETTO_DEBUG_FUNCTION_IDENTIFIER())); \
        TRACE_EVENT ("dsp", perfetto::StaticString (pf.data()) __VA_OPT__ (, ) __VA_ARGS__)

    #define TRACE_DSP_BEGIN(name) TRACE_EVENT_BEGIN ("dsp", perfetto::StaticString (name))
    #define TRACE_DSP_END() TRACE_EVENT_END ("dsp")

    // Ship a chosen public label instead of the real function name. Readable builds trace the
    // real auto-derived name (for local profiling); obfuscated (shipped) builds trace publicName.
    // publicName is a literal in your source, so a trace slice is trivial to grep back to its call
    // site, while the real name never lands in the binary and there's no decode map to keep.
    #define MELATONIN_PF_CONCAT_(a, b) a##b
    #define MELATONIN_PF_CONCAT(a, b) MELATONIN_PF_CONCAT_ (a, b)
    // unique per call site so several TRACE_DSP_PUBLIC can share one function scope (sub-regions)
    #define MELATONIN_PF MELATONIN_PF_CONCAT (melatoninTracePf_, __LINE__)
    #if PERFETTO_OBFUSCATE_DSP
        #define TRACE_DSP_PUBLIC(publicName, ...) TRACE_EVENT ("dsp", perfetto::StaticString (publicName) __VA_OPT__(,) __VA_ARGS__)
        #define TRACE_DSP_BEGIN_PUBLIC(debugName, publicName, ...) TRACE_EVENT_BEGIN ("dsp", perfetto::StaticString (publicName) __VA_OPT__(,) __VA_ARGS__)
    #else
        #define TRACE_DSP_PUBLIC(publicName, ...)                                                                                         \
            static constexpr auto MELATONIN_PF = melatonin::compileTimePrettierFunction (WRAP_COMPILE_TIME_STRING (PERFETTO_DEBUG_FUNCTION_IDENTIFIER())); \
            TRACE_EVENT ("dsp", perfetto::StaticString (MELATONIN_PF.data()) __VA_OPT__(,) __VA_ARGS__)
        #define TRACE_DSP_BEGIN_PUBLIC(debugName, publicName, ...) TRACE_EVENT_BEGIN ("dsp", perfetto::StaticString (debugName) __VA_OPT__(,) __VA_ARGS__)
    #endif

    /**
     * Tracks an instant event, which shows up as an arrow on a separate "Global Events" track in the Perfetto UI.
     * @param name Describe the event - this is what shows up in the trace.
     */
    #define TRACE_DSP_GLOBAL_INSTANT(name) TRACE_GLOBAL_INSTANT ("dsp", perfetto::StaticString (name))
#else
    #define TRACE_DSP(...)
    #define TRACE_DSP_BEGIN(name)
    #define TRACE_DSP_END()
    #define TRACE_DSP_PUBLIC(publicName, ...)
    #define TRACE_DSP_BEGIN_PUBLIC(debugName, publicName, ...)
    #define TRACE_DSP_GLOBAL_INSTANT(name)
#endif

#if PERFETTO_ENABLE_TRACE_COMPONENT
    #define TRACE_COMPONENT(...)                                                                                                             \
        static constexpr auto pf = melatonin::compileTimePrettierFunction (WRAP_COMPILE_TIME_STRING (PERFETTO_DEBUG_FUNCTION_IDENTIFIER())); \
        TRACE_EVENT ("component", perfetto::StaticString (pf.data()) __VA_OPT__ (, ) __VA_ARGS__)

    #define TRACE_COMPONENT_BEGIN(name) TRACE_EVENT_BEGIN ("component", perfetto::StaticString (name))
    #define TRACE_COMPONENT_END() TRACE_EVENT_END ("component")

    /**
     * Tracks an instant event, which shows up as an arrow on a separate "Global Events" track in the Perfetto UI.
     * @param name Describe the event - this is what shows up in the trace.
     */
    #define TRACE_COMPONENT_GLOBAL_INSTANT(name) TRACE_GLOBAL_INSTANT ("component", perfetto::StaticString (name))
#else
    #define TRACE_COMPONENT(...)
    #define TRACE_COMPONENT_BEGIN(name)
    #define TRACE_COMPONENT_END()
    #define TRACE_COMPONENT_GLOBAL_INSTANT(name)
#endif
