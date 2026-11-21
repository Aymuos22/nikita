CXX ?= c++
CPPFLAGS ?= -Iinclude
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LIBSRC = src/foundations.cpp src/graph.cpp src/optimization.cpp src/text.cpp src/network.cpp src/workbench.cpp src/domain_workbench.cpp
LIBOBJ = $(LIBSRC:src/%.cpp=build/%.o)
.PHONY: all test gui test-gui demo benchmark comparisons clean sanitize
all: build/social_cli build/social_tests build/social_bench build/benchmark_memory_tests
build:
	mkdir -p build
build/%.o: src/%.cpp include/social/algorithms.hpp include/social/network.hpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@
build/libsocial.a: $(LIBOBJ)
	$(AR) rcs $@ $^
build/social_cli: build/main.o build/libsocial.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
build/social_bench: build/benchmark.o build/benchmark_memory.o build/libsocial.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
build/test_main.o: tests/test_main.cpp include/social/algorithms.hpp include/social/network.hpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@
build/social_tests: build/test_main.o build/libsocial.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
build/benchmark_memory_tests: tests/benchmark_memory_test.cpp build/benchmark_memory.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
test: all
	./build/benchmark_memory_tests
	./build/social_tests
	python3 tests/integration.py
	python3 tests/gui_integration.py
gui: all
	python3 gui/server.py
test-gui: all
	python3 tests/gui_integration.py
benchmark: build/social_bench
	./build/social_bench results/benchmarks.csv
comparisons: all
	python3 scripts/compare_scenarios.py
demo: build/social_cli
	./build/social_cli demo
sanitize:
	$(MAKE) clean
	$(MAKE) CXXFLAGS='-std=c++17 -O1 -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined -fno-omit-frame-pointer' LDFLAGS='-fsanitize=address,undefined' test
clean:
	rm -rf build
-include $(wildcard build/*.d)
