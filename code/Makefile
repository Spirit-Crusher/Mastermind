# Compiler
CC = gcc

# Debug Flags
DEBUG_FLAGS = -g -O0

# Compilation flags
CFLAGS = -Wall -Wextra -std=c11 -Iservidor

# Conditional Debug Mode
ifeq ($(DEBUG), 1)
	CFLAGS += $(DEBUG_FLAGS)
endif
# run "make DEBUG=1" to change variable


# Default TARGET
all: servidor/JMMserv log_server/JMMlog

servidor/JMMserv: servidor/servidor_build/servidor.o servidor/servidor_build/core_gameplay.o mastermind.h
#	$(CC) $^ -o $@
	cd ./servidor && make

log_server/JMMlog: log_server/log_server_build/log_server.o log_server/log_server.h mastermind.h
#	$(CC) $^ -o $@
	cd ./log_server && make



# app server
#run_s: ./servidor/JMMserv
#	./servidor/JMMserv


# run server
run_s: ./servidor/JMMserv
	./servidor/JMMserv

# Clean build files
clean:
	rm -rf build/*.o server/*.exe

