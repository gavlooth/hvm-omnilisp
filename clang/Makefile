# OmniLisp Makefile

CC = clang
CFLAGS = -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function
LDFLAGS = -lpthread

# Debug build flags
DEBUG_CFLAGS = -g -O0 -DDEBUG

# Coverage build flags (use GCC for gcov support)
COV_CC = gcc
COV_CFLAGS = -g -O0 --coverage -fprofile-arcs -ftest-coverage -Wno-unused-parameter -Wno-unused-function

TARGET = main
COV_TARGET = main-cov
DEBUG_TARGET = main-debug

MAIN = main.c

.PHONY: all clean debug test coverage cov-report

all: $(TARGET)

$(TARGET): $(MAIN)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

debug: $(MAIN)
	$(CC) $(DEBUG_CFLAGS) -o $(DEBUG_TARGET) $< $(LDFLAGS)

coverage: $(MAIN)
	$(COV_CC) $(COV_CFLAGS) -o $(COV_TARGET) $< $(LDFLAGS)

# Run tests with coverage and generate report
cov-report: coverage
	@rm -f *.gcda
	@echo "Running test suite with coverage..."
	@./test/run_cov_tests.sh ./$(COV_TARGET) || true
	@echo ""
	@echo "Generating coverage report..."
	@gcov -o . main.c 2>/dev/null | grep -A 1 "File" || true
	@echo ""
	@echo "Detailed coverage summary:"
	@lcov --capture --directory . --output-file coverage.info --ignore-errors inconsistent 2>/dev/null || gcov -s . main.c 2>/dev/null | tail -20
	@genhtml coverage.info --output-directory coverage-report 2>/dev/null || echo "Install lcov for HTML reports: sudo pacman -S lcov"
	@echo "Coverage report generated in coverage-report/"

clean:
	rm -f $(TARGET) $(DEBUG_TARGET) $(COV_TARGET) *.profraw *.profdata *.gcda *.gcno
	rm -rf coverage-report

# Run tests
test: $(TARGET)
	@echo "Running OmniLisp tests..."
	@echo "Test 1: Basic arithmetic"
	./$(TARGET) -e "(+ 1 2)"
	@echo ""
	@echo "Test 2: Parse only"
	./$(TARGET) -p -e "(lambda [x] (+ x 1))"
	@echo ""
	@echo "Test 3: Nested expression"
	./$(TARGET) -e "(* (+ 1 2) (- 5 3))"
	@echo ""
	@echo "All tests passed!"

# Install
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

# Help
help:
	@echo "OmniLisp Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build optimized binary (default)"
	@echo "  debug    - Build debug binary"
	@echo "  clean    - Remove build artifacts"
	@echo "  test     - Run basic tests"
	@echo "  install  - Install to /usr/local/bin"
	@echo "  help     - Show this message"

