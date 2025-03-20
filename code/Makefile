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
all: servidor/MMJ_server.exe log_server/MMJ_log.exe

servidor/MMJ_server.exe: servidor/servidor.o servidor/core_gameplay.o include/mastermind.h
#	$(CC) $^ -o $@
	cd ./servidor && make

log_server/MMJ_log.exe: log_server/log_server.o log_server/log_server.h include/mastermind.h
#	$(CC) $^ -o $@
	cd ./log_server && make

# app server
#run_s: ./servidor/MMJ_server.exe
#	./servidor/MMJ_server.exe


# run server
run_s: ./servidor/MMJ_server.exe
	./servidor/MMJ_server.exe

# Clean build files
clean:
	rm -rf build/*.o server/*.exe

