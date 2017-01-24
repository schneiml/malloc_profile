A small `LD_PRELOAD`'able library that counts malloc calls. It does stacktrace collection using glibc's `backtrace(3)` and `__cxa_demangle`. It produces output for Brendan Gregg's [Flamegraph](https://github.com/brendangregg/FlameGraph).

How is that better than other profiling tools? Not at all, if you ask like that, but it should be very simple to use in the end and `backtrace` seems to work in cmssw default binaries, while e. g. `perf -g` does not.

Usage
-----

Set the `LD_PRELOAD` environment variable to the name of the libray (`libmalloc_profile.so`). A full path might be required. Then run the program you want to profile as normal. If it is dynamically linked as usual you should see some printouts by the library on the first `malloc` call, and the library starts recording. When the program terminates (and did not crash too bad) you should see a `malloc.prof.$PID` file in the working directory (if you don't see one, it might also be that no event was recorded, which is the case if less than `skip_number` calls are caught, to avoid polluting the directory with outputs for short-lived children). 

The output file can be directly passed to `flamegraph.pl`.


Options
-------


`skip_number` average amount of calls to ignore between two stack trace extractions. This will be randomized to get a good sample. At the moment it has to be a power of 2, to avoid strange problems with % (modulo).

`record_time` if `true`, try to estimate (wallclock) time instead of counting malloc calls. This will obviously fail when there are no malloc calls in the code, but it should work as long as there are enough of them. The highest levels of the flame graph will be nonsense, but there you better use perf anyways. It does take the overhead into account, so even if your program runs much slower due to a small `skip_number`, the estimation should be fine. (This does not work for things like IO, of course.)


The options can be set in the source code.

TODO
----

Every stack trace extraction might take 1000 malloc calls itself, so there definitly is a lot of overhead. This also explains the large values for `skip_number`. A better approach might be dumping the addresses and doing the unmanglig later, but as-is it is much easier (and there will be overhead in any case).

Multi-threaded programs might not be handled correctly. It should not crash and the counts should be fine, but time estimation would have to take the thread ID into account.
