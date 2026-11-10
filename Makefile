CXX      ?= g++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wpedantic -O2 -Iinclude
SRC      := $(wildcard src/*.cpp)
TEST_SRC := $(wildcard tests/*.cpp)

.PHONY: all test demo manual clean

all: elevator_sim elevator_tests

elevator_sim: $(SRC) app/main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

elevator_tests: $(SRC) $(TEST_SRC) tests/test_common.h
	$(CXX) $(CXXFLAGS) $(SRC) $(TEST_SRC) -o $@

test: elevator_tests
	./elevator_tests

demo: elevator_sim
	./elevator_sim --demo

manual: elevator_sim
	./elevator_sim < tests/sample_manual_input.txt

clean:
	rm -f elevator_sim elevator_tests elevator_state.csv test_*.tmp.csv
