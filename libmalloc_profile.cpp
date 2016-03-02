#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <signal.h>
#include <execinfo.h>
#include <regex>
#include <cxxabi.h>
#include <sstream>

class StackCollector {
    std::regex rx_trace = std::regex("(.*)\\((.+)\\+0x.*\\).*");
    std::map<std::string, int> all;

public:
    void add_stackframe() {
        if (rand() % 1024 != 1) return;
        void *trace[16];
        char **messages = (char **)NULL;
        int i, trace_size = 0;

        trace_size = backtrace(trace, 16);
        messages = backtrace_symbols(trace, trace_size);
        
        
        std::stringstream bt;
        for (i=trace_size-1; i >= 2; --i) {
            std::cmatch results;
            int r = 0;
            if (std::regex_match(messages[i], results, rx_trace)) {
                char * demangled = abi::__cxa_demangle(results[2].str().c_str(), 0, 0, &r);
                if (r == 0) {
                    bt << std::string(demangled) << ";";
                } else {
                    bt << results[2].str() << ";";
                }
                free(demangled);
            }
        }
//         printf("%s\n", bt.str().c_str());
        all[bt.str()]++;
        free(messages);
    }
    
    void dump() {
        for (auto stack : all) {
            printf("%s %d\n", stack.first.c_str(), stack.second);
        }
    }
    
    ~StackCollector() {
        dump();
    }
};



static void* (*old_malloc)(size_t size) = NULL;
static bool in_malloc = false;
static StackCollector sc;


void* malloc(size_t size)
{
    if (!in_malloc) {
        in_malloc = true;
        char *msg;

        if (old_malloc == NULL) {
                fprintf(stderr, "malloc : wrapping malloc\n");
                fflush(stderr);
                // next_ioctl = dlsym((void *) -11, /* RTLD_NEXT, */ "ioctl");
                old_malloc = dlsym(RTLD_NEXT, "malloc");
                fprintf(stderr, "old_malloc = %p\n", old_malloc);
                fflush(stderr);
                if ((msg = dlerror()) != NULL) {
                        fprintf(stderr, "malloc: dlopen failed : %s\n", msg);
                        fflush(stderr);
                        exit(1);
                } else
                        fprintf(stderr, "malloc: wrapping done\n");
                fflush(stderr);
        }
        sc.add_stackframe();
        in_malloc = false;
    }
    return old_malloc(size);
}
