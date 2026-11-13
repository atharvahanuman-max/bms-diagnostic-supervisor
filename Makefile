# BMS Diagnostic Supervisor - build and Google Test rules

CC = gcc
CXX = g++

BUILD_DIR := build

CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Iinclude -MMD -MP
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Iinclude -MMD -MP
TEST_LIBS ?= -lgtest -lgtest_main -pthread

APP_SOURCES := $(wildcard src/*.c)
APP_OBJECTS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(APP_SOURCES))

# The test executable must link every application module except main.c.
LIB_OBJECTS := $(filter-out $(BUILD_DIR)/main.o,$(APP_OBJECTS))

TEST_SOURCES := $(wildcard tests/*.cpp)
TEST_OBJECTS := $(patsubst tests/%.cpp,$(BUILD_DIR)/tests/%.o,$(TEST_SOURCES))

APP := $(BUILD_DIR)/bms
TEST_APP := $(BUILD_DIR)/bms_tests

.PHONY: all run test clean

all: $(APP)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile the C application modules.
$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile Google Test source files as C++.
$(BUILD_DIR)/tests/%.o: tests/%.cpp | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link the normal application, including main.o.
$(APP): $(APP_OBJECTS)
	$(CC) $^ -o $@ -lm

# Link tests with the C modules, deliberately excluding main.o.
$(TEST_APP): $(TEST_OBJECTS) $(LIB_OBJECTS)
	$(CXX) $^ -o $@ $(TEST_LIBS) -lm

run: $(APP)
	./$(APP)

test: $(TEST_APP)
	./$(TEST_APP)

clean:
	rm -rf $(BUILD_DIR)

# Rebuild objects when included headers change.
-include $(APP_OBJECTS:.o=.d)
-include $(TEST_OBJECTS:.o=.d)
