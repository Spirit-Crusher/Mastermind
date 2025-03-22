## MAKEFILE para o servidor do jogo Mastermind
# Compiler
CC = gcc

# Debug Flags
DEBUG_FLAGS = -g -O0

# Compilation flags
CFLAGS = -Wall -Wextra -std=c11 -D_XOPEN_SOURCE=500 -Iservidor -lrt -lpthread

# Conditional Debug Mode
ifeq ($(DEBUG), 1)
	CFLAGS += $(DEBUG_FLAGS)
endif

# Default TARGET
default: ./JMM_server.exe

# Run Default TARGET
run: default
	JMM_server.exe

servidor_build/servidor.o: servidor.c servidor.h
	@mkdir -p servidor_build
	$(CC) $(CFLAGS) -c $< -o $@

servidor_build/core_gameplay.o: core_gameplay.c core_gameplay.h
	@mkdir -p servidor_build
	$(CC) $(CFLAGS) -c $< -o $@

JMM_server.exe: servidor_build/servidor.o servidor_build/core_gameplay.o ../include/mastermind.h
#	@mkdir -p bin
	$(CC) $^ -o $@

#test: log_server_test.c log_server.h
#	$(CC) $(CFLAGS) $< -o $@.exe
