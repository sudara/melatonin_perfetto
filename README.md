## Melatonin Perfetto

Sounds like an ice cream flavor (albeit a sleepy one).

However, it's just a way to use the amazing [Perfetto](http://perfetto.dev) performance tracing in JUCE.

![image-1024x396](https://user-images.githubusercontent.com/472/180338251-ce3c5814-ff9c-4fbb-a8c0-9caefc2f34dc.png)

## Why Perfetto?

The short answer: It lets you acurately measure different parts of your code and visualize it over time.

When you have performance concerns, general profiling is a great first step, and one worth famaliarizing yourself with. But it's very hand-wavey, pretty much only good for drunkenly pointing you to "the one hotspot, I think". Everything is aggregate. Everything is relative. 

Perfetto lets you get down the cold hard absolute facts: which functions are take how many µs/ms. How often are those functions being called, how does it look across time.

Perfetto is perfect for measuring paint calls because you can see how many times they are occur and what the timing is for each occurance. 

Perfetto is the successor to [`chrome://tracing`](https://slack.engineering/chrome-tracing-for-fun-and-profit/). 


## Using from a CMake project

This project supports being added to CMake projects using several methods:
- `find_package`
- `FetchContent`
- `add_subdirectory`

### `find_package`

You can install this module to your system by cloning the code and then running the following commands:
```sh
cmake -B Builds
cmake --build Builds
cmake --install Builds
```

The `--install` command will write to system directories, so it may require `sudo`.

Once this module is installed to your system, you can simply add to your CMake project:
```cmake
find_package (MelatoninPerfetto)

target_link_libraries (yourTarget PRIVATE Melatonin::melatonin_perfetto)
```

### `FetchContent`

Here is an example usage:
```cmake
include (FetchContent)

FetchContent_Declare (melatonin_perfetto
                      GIT_REPOSITORY https://github.com/sudara/melatonin_perfetto.git
                      GIT_TAG origin/main)

FetchContent_MakeAvailable (melatonin_perfetto)

target_link_libraries (yourTarget PRIVATE Melatonin::melatonin_perfetto)
```

### `add_subdirectory`

You can also add this repository as a git submodule to your project, and simply call `add_subdirectory` on it.


Notice that in all cases, the exported target you should link against is exactly the same, `Melatonin::melatonin_perfetto`.


## How to use

### Add a few pieces of guarded code to your plugin
Put this in PluginProcessor's constructor:

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
And as a member of the plugin processor:
```
#if PERFETTO
    std::unique_ptr<perfetto::TracingSession> tracingSession;
#endif
```
### Pepper around some sweet sweet macros

Unlike with profiling, you'll have to opt functions in to being measured

Pepper around `TRACE_DSP()` or `TRACE_COMPONENT()` macros. 

Or use perfetto's raw `TRACE_EVENT()` 

### Enable profiling

When you want to profile, set `#define PERFETTO 1` (somewhere before you first include the module, as a preprocessor directive, or just toggle in the module header if you are lazy like me). That'll actually include the google lib. When it's not defined, the `TRACE_DSP` calls will be no-ops, so you can just leave them in place with no impact to your app.

### Run your app and perform the actions you want traced

When you quit your app, a trace file will be dumped 

**(Note: don't just terminate it via your IDE, the file will be only dumped on a graceful quit)**.

Find the trace file and drag it into https://ui.perfetto.dev

### That's it

You can keep the macros peppered around in your app during normal dev/release. Just remember to `#define PERFETTO 0` so everything gets turned into a no-op. 

That's handy for future you!

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
