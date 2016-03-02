#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <signal.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <sstream>
#include <fstream>
#include <map>
#include <regex>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>


class StackCollector {
    std::regex rx_trace = std::regex("(.*)\\((.+)\\+0x.*\\).*");
    std::map<std::string, int64_t> *all = nullptr;
    std::mutex map_mutex;
    const bool record_time = true;
    const int skip_number = 32*1024;
    std::atomic<int> skip_counter{skip_number};
    std::chrono::high_resolution_clock::time_point last_ev = 
    	std::chrono::high_resolution_clock::now();

public:
    void add_stackframe() {
        if (skip_counter-- != 0) return;
	// record time before in the very start to not measure overhead. 
	// TODO: time is broken for MT anyways...
        auto start_time = std::chrono::high_resolution_clock::now();
	

	std::lock_guard<std::mutex> lock(map_mutex);

	// Be very careful here... seems to throw a FPE with %?
	int new_skip = rand();
	new_skip = new_skip & (2*skip_number-1);
	skip_counter = new_skip;
	
	if (all == nullptr) {
	  all = new std::map<std::string, int64_t>();
	}
        void *trace[64];
        char **messages = (char **)NULL;
        int i, trace_size = 0;

        trace_size = backtrace(trace, 64);
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
	//const char* str_key = bt.str().c_str();
        //printf("%s\n", str_key);
	if (record_time) {
	    std::chrono::duration<double> time_span = 
	        std::chrono::duration_cast<std::chrono::duration<double>>(start_time - last_ev);
	    (*all)[bt.str()] += (int64_t) (time_span.count() * 1e6);
            last_ev = std::chrono::high_resolution_clock::now();
	} else {
            ++((*all)[bt.str()]);
	}
        free(messages);
    }
    
    void dump() {
	if (!all || all->size() == 0) return;
        pid_t pid = getpid();
      	std::ofstream out("malloc.prof." + std::to_string(pid));
        for (auto stack : *all) {
            //printf("%s %d\n", stack.first.c_str(), stack.second);
	    out << stack.first << " " << stack.second << std::endl;
        }
	delete all;
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
	  	srand(37);
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
