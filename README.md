A small LD_PRELOAD'able library that counts malloc calls. It does stacktrace collection using glibc's `backtrace(3)` and `__cxa_demangle`. It produces output for Brendan Gregg's [Flamegraph](https://github.com/brendangregg/FlameGraph).

How is that better than other profiling tools? Not at all, if you ask like that, but it should be very simple to use in the end and `backtrace` seems to work in cmssw defualt binaries, while e. g. perf does not.
