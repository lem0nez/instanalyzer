-include libs.make

CXX = g++
WARNINGS = -Wall -Wextra -fexceptions
DEFINES = -std=c++17 -lstdc++fs -ggdb

BUILD = build
OBJ = $(BUILD)/obj
SRC = src
LIB = lib

THREADS = 5

SOURCES = $(wildcard $(SRC)/*.cpp)
OBJECTS = $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCES))

.DEFAULT_GOAL = $(BUILD)/instanalyzer
.DEFAULT: $(BUILD)/instanalyzer

$(BUILD)/instanalyzer: $(OBJECTS)
	$(CXX) $(WARNINGS) $^ -o $@ $(LDFLAGS) $(DEFINES)

$(OBJ)/%.o: $(SRC)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(WARNINGS) -c $< -o $@ $(CXXFLAGS) $(DEFINES)

.PHONY: clean
clean:
	rm -rf $(BUILD)
