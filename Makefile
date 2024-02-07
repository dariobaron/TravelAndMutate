
# list of all modules
SOURCES := $(wildcard src/*.cpp)
# list of all headers
HEADERS := $(wildcard src/*.hpp)
# extension of the modules
MODULEEXTENSION := $(shell python3-config --extension-suffix)
# list of all object files by sostitution of all the *.cpp with *.o
COMPILEDMODULES := $(patsubst src/%.cpp,TravelAndMutate/%$(MODULEEXTENSION),$(SOURCES))
# list of module names
MODULENAMES := $(patsubst src/%.cpp,%,$(SOURCES))

# compiler directives
WARNING := -Wall -Wpedantic -Wextra -Wno-sign-compare
CXX := g++
CXXFLAGS := -O3 -shared -std=c++20 -fPIC
INCLUDES := $(shell python3 -m pybind11 --includes) -I$(shell python -c "import numpy; print(numpy.get_include())")


.PHONY: all clean $(MODULENAMES)

all: $(COMPILEDMODULES)

$(MODULENAMES): % : TravelAndMutate/%$(MODULEEXTENSION)

%$(MODULEEXTENSION): ../src/%.cpp $(HEADERS)
	$(CXX) $(WARNING) $(CXXFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -f $(COMPILEDMODULES)
