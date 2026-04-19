CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pedantic -Iinclude
BUILD_DIR := build
BUILD_STAMP := $(BUILD_DIR)/.dir
OBJS := \
	$(BUILD_DIR)/term.o \
	$(BUILD_DIR)/parser.o \
	$(BUILD_DIR)/church.o \
	$(BUILD_DIR)/eval_student.o \
	$(BUILD_DIR)/factorial_student.o \
	$(BUILD_DIR)/ackermann_student.o

.PHONY: build test clean

build: $(OBJS)

$(BUILD_STAMP):
	mkdir -p $(BUILD_DIR)
	touch $(BUILD_STAMP)

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_STAMP)
	$(CXX) $(CXXFLAGS) -c $< -g -o $@

test: build
	test -n "$(TEST_FILE)"
	$(CXX) $(CXXFLAGS) "$(TEST_FILE)" $(OBJS) -o $(BUILD_DIR)/test_runner
	./$(BUILD_DIR)/test_runner

clean:
	rm -rf $(BUILD_DIR)
