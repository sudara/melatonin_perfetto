/*
BEGIN_JUCE_MODULE_DECLARATION

 ID:               melatonin_perfetto
 vendor:           Sudara
 version:          1.0.0
 name:             Melatonin Perfetto
 description:      Perfetto module for JUCE
 license:          MIT
 dependencies:     juce_core

END_JUCE_MODULE_DECLARATION
*/

#ifndef PERFETTO
    #define PERFETTO 0
#endif

#pragma once

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
        session = perfetto::Tracing::NewTrace();
        session->Setup (cfg);
        session->StartBlocking();
    }

    void endSession()
    {
        // Make sure the last event is closed for this example.
        perfetto::TrackEvent::Flush();

        // Stop tracing
        session->StopBlocking();

        writeFile();
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

    void writeFile()
    {
        // Read trace data
        std::vector<char> trace_data (session->ReadTraceBlocking());

    #if defined(_MSC_VER)
        auto file = juce::File::getSpecialLocation (juce::File::SpecialLocationType::userDesktopDirectory);
    #else
        auto file = juce::File::getSpecialLocation (juce::File::SpecialLocationType::userHomeDirectory).getChildFile ("Downloads");
    #endif
        auto currentTime = juce::Time::getCurrentTime().formatted ("%Y-%m-%d_%H%M");
        auto output = file.getChildFile ("perfetto-" + currentTime + ".pftrace").createOutputStream();
        output->setPosition (0);
        output->write (&trace_data[0], trace_data.size() * sizeof (char));
    }

    std::unique_ptr<perfetto::TracingSession> session;
};

    #define TRACE_DSP(...) TRACE_EVENT ("dsp", PERFETTO_DEBUG_FUNCTION_IDENTIFIER(), ##__VA_ARGS__)
    #define TRACE_COMPONENT(...) TRACE_EVENT ("component", PERFETTO_DEBUG_FUNCTION_IDENTIFIER(), ##__VA_ARGS__)

#else // if PERFETTO
    #define TRACE_EVENT_BEGIN(category, ...)
    #define TRACE_EVENT_END(category)
    #define TRACE_EVENT(category, ...)
    #define TRACE_DSP(...)
    #define TRACE_COMPONENT(...)
#endif
