# Compiler
CC = gcc

# Debug Flags
DEBUG_FLAGS = -g -O0

# Compilation flags
CFLAGS = -Wall -Wextra -std=c11 -Iinclude -Iservidor

# Conditional Debug Mode
ifeq ($(DEBUG), 1)
	CFLAGS += $(DEBUG_FLAGS)
endif
# run "make DEBUG=1" to change variable


# Default TARGET
all: bin/server.exe

# Default TARGET
run: all
	./bin/server.exe

build/servidor.o: servidor/servidor.c servidor/core_gameplay.h include/mastermind.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/core_gameplay.o: servidor/core_gameplay.c servidor/core_gameplay.h include/mastermind.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

bin/server.exe: build/core_gameplay.o build/servidor.o 
	@mkdir -p bin
	$(CC) $^ -o $@

test: test/core_gameplay_test.c build/core_gameplay.o
	@mkdir -p test
	$(CC) $(CFLAGS) $^ -o test/test.exe
	./test/test.exe



# Clean build files
clean:
	rm -rf build/*.o server/*.exe

