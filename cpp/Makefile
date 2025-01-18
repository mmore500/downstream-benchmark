CXX ?= g++
CXXCLANG ?= clang++

CFLAGS_all := -Wall -Wno-unused-function -std=c++23 -I. -march=native
CFLAGS_nat := -O3 -DNDEBUG $(CFLAGS_all)
CFLAGS_nat_debug := -g $(CFLAGS_all)

HEADERS := $(shell find . -name '*.hpp')

MAIN_BIN := ./main

default: release

.PHONY: all clean check debug default release run-release run-debug
all: release
debug: CFLAGS_nat := $(CFLAGS_nat_debug)
debug: release

release: $(MAIN_BIN)

check:
	@echo "Checking C++23 compatibility..."
	@for file in $(HEADERS) $(MAIN_BIN).cpp; do \
		echo "Checking $$file with GCC..."; \
		$(CXX) $(CFLAGS_nat) -fsyntax-only "$$file" || exit 1; \
		if command -v $(CXXCLANG) > /dev/null 2>&1; then \
			echo "Checking $$file with Clang..."; \
			$(CXXCLANG) $(CFLAGS_nat) -fsyntax-only "$$file" || exit 1; \
		fi \
	done
	@echo "All files pass C++23 syntax check"

$(MAIN_BIN): $(MAIN_BIN).cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS_nat) $< -o $@

run-release: release
	@echo "Running release build..."
	$(MAIN_BIN)

run-debug: debug
	@echo "Running debug build..."
	$(MAIN_BIN)

clean:
	@echo "Cleaning build artifacts..."
	rm -f $(MAIN_BIN)
