CXX= g++
CXXFLAGS=-w
LDFLAGS = -lpthread

all: radixsort radixsort-parallel

radixsort:main.cc Sorters.hh RadixSorter.cc HRTimer.hh
	$(CXX) $(CXXFLAGS) main.cc RadixSorter.cc -o radixsort $(LDFLAGS)
radixsort-parallel: main.cc Sorters.hh RadixSorter.cc HRTimer.hh
	$(CXX) $(CXXFLAGS) -DPARALLEL main.cc RadixSorter.cc -o radixsort-parallel $(LDFLAGS)

clean:
	rm -f radixsort
	rm -f radixsort-parallel