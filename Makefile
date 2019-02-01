-include libs.make

CXX = @echo '  g++    $@' && g++
WARNINGS = -Wall -fexceptions -Wextra -Wno-psabi
DEFINES = -std=c++17 -lstdc++fs -ggdb -pthread

BUILD = build
OBJ = $(BUILD)/obj
SRC = src
LIB = lib

PROJECT_PATH = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
THREADS = 5

test:
	@echo $(MAKEFILE_PATH)

SOURCES = $(wildcard $(SRC)/*.cpp)
OBJECTS = $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCES))

.DEFAULT_GOAL = $(BUILD)/instanalyzer
.DEFAULT: $(BUILD)/instanalyzer

$(BUILD)/instanalyzer: $(OBJECTS)
	$(CXX) $(WARNINGS) $^ -o $@ $(LDFLAGS) $(LDLIBS) $(DEFINES)

$(OBJ)/%.o: $(SRC)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(WARNINGS) -c $< -o $@ $(CXXFLAGS) $(DEFINES)

.PHONY: clean
clean:
	rm -rf autom4te.cache $(BUILD) config.log config.status
