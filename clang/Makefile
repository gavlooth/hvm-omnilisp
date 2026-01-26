# OmniLisp Makefile

CC = clang
CFLAGS = -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function
LDFLAGS = -lpthread

# Debug build flags
DEBUG_CFLAGS = -g -O0 -DDEBUG

TARGET = omnilisp
DEBUG_TARGET = omnilisp-debug

MAIN = main.c

.PHONY: all clean debug test

all: $(TARGET)

$(TARGET): $(MAIN)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

debug: $(MAIN)
	$(CC) $(DEBUG_CFLAGS) -o $(DEBUG_TARGET) $< $(LDFLAGS)

clean:
	rm -f $(TARGET) $(DEBUG_TARGET)

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

