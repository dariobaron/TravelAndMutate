# shell selection
SHELL := /bin/bash

# compiler
CXX := g++

# Python
PYTHON_COMMAND := python
PYTHON_CONFIG_COMMAND := python-config

# paths
PYTHON_ENV_DIR := env/
SOURCE_DIR := src/
BUILD_DIR := build/
TARGET_DIR := TravelAndMutate/
EXTERN_INCLUDE_PATH := mixlightlib/

# compilation flags
COMPILATION_FLAGS := -std=c++20 -O3 -fPIC -shared -fconcepts-diagnostics-depth=2 -fvisibility=hidden -MMD -MP
WARNINGS := -Wall -Wextra -Werror -pedantic -pedantic-errors -Wpedantic -Wno-sign-compare

# necessary flags:
MODULE_EXTENSION := $(shell $(PYTHON_CONFIG_COMMAND) --extension-suffix)
PYBIND11_INCLUDE_PATH := $(shell $(PYTHON_COMMAND) -m pybind11 --includes)
NUMPY_INCLUDE_PATH := $(shell $(PYTHON_COMMAND) -c "import numpy; print(numpy.get_include())")
PYTHON_RUNTIME_LIB := $(shell $(PYTHON_CONFIG_COMMAND) --ldflags)
INCLUDE_PATH := $(PYBIND11_INCLUDE_PATH) -I$(NUMPY_INCLUDE_PATH) $(foreach dir, $(EXTERN_INCLUDE_PATH),-I$(dir))
LINKING_LIBS := $(PYTHON_RUNTIME_LIB)
CXXFLAGS := $(COMPILATION_FLAGS) $(WARNINGS) $(INCLUDE_PATH) $(LINKING_LIBS)

# files
SOURCES := $(wildcard $(SOURCE_DIR)*.cpp)
DEPENDENCIES := $(patsubst $(SOURCE_DIR)%.cpp, $(BUILD_DIR)%.d, $(SOURCES))
TARGETS := $(patsubst $(SOURCE_DIR)%.cpp, $(TARGET_DIR)%$(MODULE_EXTENSION), $(SOURCES))
MODULE_NAMES := $(patsubst $(SOURCE_DIR)%.cpp, %, $(SOURCES))

# Makefile
.PHONY: all $(MODULE_NAMES) setup clean

all: $(BUILD_DIR) $(TARGETS)

$(BUILD_DIR):
	mkdir -p $(SOURCE_DIR) $(BUILD_DIR) $(TARGET_DIR)

$(MODULE_NAMES): %: $(TARGET_DIR)%$(MODULE_EXTENSION)

$(TARGET_DIR)%$(MODULE_EXTENSION): $(SOURCE_DIR)%.cpp
	$(CXX) $(CXXFLAGS) -MF $(BUILD_DIR)$*.d -o $@ $<

-include $(DEPENDENCIES)

setup: createvenv installlibs initrepo

installlibs: createvenv
	. $(PYTHON_ENV_DIR)bin/activate && $(PYTHON_COMMAND) -m pip install -r requirements.txt

createvenv:
	if [ ! -d $(PYTHON_ENV_DIR) ]; then $(PYTHON_COMMAND) -m venv $(PYTHON_ENV_DIR); fi

initrepo: setupgitfilters setupsubmodules

setupgitfilters:
	git config filter.strip-notebook-output.clean 'jupyter nbconvert --ClearOutputPreprocessor.enabled=True --to=notebook --stdin --stdout --log-level=ERROR'; \
	git config filter.strip-notebook-output.smudge 'cat'; \
	git config filter.strip-notebook-output.required true

setupsubmodules:
	git config diff.submodule log; \
	git config submodule.recurse true

clean:
	rm -rf $(TARGETS) $(BUILD_DIR)