## Melatonin Perfetto

Sounds like an ice cream flavor (albeit a sleepy one).

However, it's just a way to use the amazing [Perfetto](http://perfetto.dev) performance tracing in JUCE.

![image-1024x396](https://user-images.githubusercontent.com/472/180338251-ce3c5814-ff9c-4fbb-a8c0-9caefc2f34dc.png)

## Why Perfetto?

Perfetto is the successor to [`chrome://tracing`](https://slack.engineering/chrome-tracing-for-fun-and-profit/). 

## How to install

You can add this project as a git submodules:

```
git submodule add -b main https://github.com/sudara/melatonin_perfetto.git modules/melatonin_perfetto
```

The CMake setup for Perfetto itself is a bit hairy, not going to lie. 

I had to dig deep to get it happy on MSVC. This is what my working config looks like:

```
Include(FetchContent)
FetchContent_Declare(
    Perfetto
    GIT_REPOSITORY https://android.googlesource.com/platform/external/perfetto
    GIT_TAG v25.0)
FetchContent_Populate(Perfetto)
include_directories(${perfetto_SOURCE_DIR}/sdk)
add_library(perfetto STATIC ${perfetto_SOURCE_DIR}/sdk/perfetto.cc)
target_compile_definitions(perfetto PUBLIC NOMINMAX=1 WIN32_LEAN_AND_MEAN=1)
set_target_properties(perfetto PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
if(MSVC)
    target_compile_options(perfetto PRIVATE "/bigobj")
endif()
```

Then you can add this project as a JUCE module:

```
juce_add_module("modules/melatonin_perfetto")
```

Don't forget to link perfetto to your plugin target too:
```
target_link_libraries(YourPlugin PRIVATE Perfetto)
```

Phew. That was the hard part.

## How to use

1. Put this in PluginProcessor's constructor:

```
#if PERFETTO
    MelatoninPerfetto::get().beginSession();
#endif
```
and in your destructor:
```
#if PERFETTO
    MelatoninPerfetto::get().endSession();
#endif
```
3. Pepper around `TRACE_DSP()` or `TRACE_COMPONENT()` macros in a function to add that category of event to the trace. (See below for more options).
4. When you want to profile, set `#define PERFETTO 1` (somewhere before you first include the module, as a preprocessor directive, or just toggle in the module header if you are lazy like me). That'll actually include the google lib. When it's not defined, the `TRACE_DSP` calls will be no-ops, so you can just leave them in place with no impact to your app.
5. Run your app
6. Quit your app and a trace file will be dumped **(Note: don't just terminate it via your IDE, the file will be only dumped on a graceful quit)**.
7. Find the trace file and drag it into https://ui.perfetto.dev

You can keep the macros peppered around in your app, they are defined to do nothing when `#define PERFETTO 0`. That's handy for future you!

## Options

By default, there are two "categories" defined, dsp and components. 

The current fuction name is passed as the name. You can add custom parameters if you need to further differentiate. 

You can also use the built in `TRACE_EVENT` which takes a name if you don't want it to derive a name based on the function.

Go wild! 

```
TRACE_DSP("startSample", startSample, "numSamples", numSamples);
```

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
