## MAKEFILE for log_server.exe and it's tests

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

# Default TARGET
default: ./MMJ_log.exe

# Run Default TARGET
run: default
	./MMJ_log.exe

build/log_server.o: log_server.c log_server.h include/mastermind.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

./MMJ_log.exe: log_server.o
	@mkdir -p bin
	$(CC) $^ -o $@
