## Melatonin Perfetto

Sounds like an ice cream flavor (albeit a sleepy one).

However, it's just a way to use the amazing [Perfetto](http://perfetto.dev) performance tracing in JUCE.

## Why Perfetto?

Perfetto is the successor to [`chrome://tracing`](https://slack.engineering/chrome-tracing-for-fun-and-profit/). 

## How to use

* By default, you don't want perfetto on
* When you do, set `#define PERFETTO 1` somewhere before including the module.
* Pepper around `TRACE_DSP()` or `TRACE_COMPONENT()` macros in a function to add that category of event to the trace. (See below for more options).
* Run your app
* Quit your app to get the trace **(Note: don't just terminate it via your IDE, the file will be only dumped on a graceful quit)**.
* Find the trace and stick it into https://ui.perfetto.dev

## Options

TRACE_DSP("startSample", startSample, "numSamples", numSamples);

## Assumptions / Caveats

* On Mac, the trace is dumped to your Downloads folder. On Windows, it's dumped to your Desktop (sorry not sorry).
* Traces are set to in memory, 8MB max


## Misc

Get warned by your tests if someone left `PERFETTO=1` on with a test like this (this example is Catch2):

```c++
TEST_CASE ("Perfetto not accidentally left enabled", "[perfetto]")
{
#if defined(PERFETTO) && PERFETTO
    FAIL_CHECK ("PERFETTO IS ENABLED");
#else
    SUCCEED ("PERFETTO DISABLED");
#endif
}
```
