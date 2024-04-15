
# list of all modules
SOURCES := $(wildcard src/*.cpp)
# list of all headers
HEADERS := $(wildcard src/*.hpp src/*/*.hpp src/*/*/*.hpp)
# extension of the modules
MODULEEXTENSION := $(shell python-config --extension-suffix)
PYTHONRUNTIMELIB := $(shell python-config --ldflags)
# list of all object files by sostitution of all the *.cpp with *.o
COMPILEDMODULES := $(patsubst src/%.cpp,TravelAndMutate/%$(MODULEEXTENSION),$(SOURCES))
# list of module names
MODULENAMES := $(patsubst src/%.cpp,%,$(SOURCES))

# compiler directives
WARNING := -Wall -Wpedantic -Wextra -Wno-sign-compare
CXX := g++
CXXFLAGS := -O3 -shared -std=c++20 -fPIC -fconcepts-diagnostics-depth=2 -fvisibility=hidden
INCLUDES := $(shell python -m pybind11 --includes) -I$(shell python -c "import numpy; print(numpy.get_include())")


.PHONY: all install clean $(MODULENAMES) prova prova2

all: $(COMPILEDMODULES)

$(MODULENAMES): % : TravelAndMutate/%$(MODULEEXTENSION)

%$(MODULEEXTENSION): ../src/%.cpp $(HEADERS)
	$(CXX) $(WARNING) $(CXXFLAGS) $(INCLUDES) $< -o $@

install:
	python -m venv env/;\
	. env/bin/activate;\
	python -m pip install -r requirements.txt

clean:
	rm -f $(COMPILEDMODULES)

prova: 
	$(CXX) $(WARNING) $(CXXFLAGS) $(INCLUDES) src/tmp/prova.cpp -o TravelAndMutate/prova$(MODULEEXTENSION)

prova2:
	$(CXX) $(WARNING) -O3 -std=c++20 src/tmp/prova2.cpp -o src/tmp/prova2.x