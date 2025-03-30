## MAKEFILE para a aplicação do jogo Mastermind
# Compiler
CC = gcc

# Debug Flags
DEBUG_FLAGS = -g -O0

# Compilation flags
CFLAGS = -Wall -Wextra -std=c11 -D_XOPEN_SOURCE=500 -lrt -lpthread

# Conditional Debug Mode
ifeq ($(DEBUG), 1)
	CFLAGS += $(DEBUG_FLAGS)
endif

# Default TARGET
default: ./JMMapl

# Run Default TARGET
run: default
	./JMMapl

client_build/monitor.o: monitor.c cliente.h
	@mkdir -p client_build
	$(CC) $(CFLAGS) -c $< -o $@

client_build/comandos.o: comandos.c cliente.h
	@mkdir -p client_build
	$(CC) $(CFLAGS) -c $< -o $@

client_build/JMMapl.o: JMMapl.c cliente.h
	@mkdir -p client_build
	$(CC) $(CFLAGS) -c $< -o $@

JMMapl: client_build/monitor.o client_build/comandos.o client_build/JMMapl.o ../mastermind.h
#	@mkdir -p bin
	$(CC) $^ -o $@

#test: log_server_test.c log_server.h
#	$(CC) $(CFLAGS) $< -o $@.exe

clean:
	rm -rf client_build/*.o JMMapl
