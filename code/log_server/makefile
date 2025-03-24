## MAKEFILE for log_server.exe and it's tests

# Compiler
CC = gcc

# Debug Flags
DEBUG_FLAGS = -g -O0

# Compilation flags
CFLAGS = -Wall -Wextra -std=c11 -D_XOPEN_SOURCE=500 -Iservidor -lrt -lpthread
## -D_XOPEN_SOURCE=500 devido a um erro do "implicit declaration of function ‘ftruncate’ "
## NÃO ESQUECER as FLAGS -lrt -lpthread ---------------------------------------------------------------------------------

# Conditional Debug Mode
ifeq ($(DEBUG), 1)
	CFLAGS += $(DEBUG_FLAGS)
endif

# Default TARGET
default: ./JMM_log.exe

# Run Default TARGET
run: default
	./JMM_log.exe

log_server_build/log_server.o: log_server.c log_server.h ../include/mastermind.h
	@mkdir -p log_server_build
	$(CC) $(CFLAGS) -c $< -o $@

./JMM_log.exe: log_server_build/log_server.o
#	@mkdir -p bin
	$(CC) $^ -o $@

test: log_server_test.c log_server.h
	$(CC) $(CFLAGS) $< -o $@.exe