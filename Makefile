 #
 # Makefile
 #

all:    libs

libs:   libmalloc_profile.so

libmalloc_profile.so:       libmalloc_profile.cpp
	rm -f libmalloc_profile.so*
	g++ --std=gnu++11 -g -O2 -fPIC -fpermissive -shared -Wl,-soname,libmalloc_profile.so.1 -ldl -o libmalloc_profile.so.1.0  libmalloc_profile.cpp 
	ln -s libmalloc_profile.so.1.0 libmalloc_profile.so.1
	ln -s libmalloc_profile.so.1 libmalloc_profile.so

clean:
	rm -f libmalloc_profile.so* 
