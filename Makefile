CXX = g++
WARNINGS = -Wall -Wextra -fexceptions
DEFINES = -std=c++17 -lstdc++fs -ggdb

BUILD = build
OBJ = $(BUILD)/obj
LIB = lib
SRC = src

SOURCES = $(wildcard $(SRC)/*.cpp)
OBJECTS = $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCES))

.DEFAULT: $(BUILD)/instanalyzer

$(BUILD)/instanalyzer: $(OBJECTS)
	$(CXX) $(WARNINGS) $^ -o $@ $(DEFINES)

$(OBJ)/%.o: $(SRC)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(WARNINGS) -c $< -o $@ $(DEFINES)

.PHONY: clean

clean:
	rm -rf $(BUILD)
