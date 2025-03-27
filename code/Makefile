
# Default TARGET
all: servidor/JMMserv log_server/JMMlog log_server/JMMapl

servidor/JMMserv: 
#	$(CC) $^ -o $@
	cd ./servidor && make

log_server/JMMlog: 
#	$(CC) $^ -o $@
	cd ./log_server && make

log_server/JMMapl: 
#	$(CC) $^ -o $@
	cd ./client && make



# run server
run_s: ./servidor/JMMserv
	./servidor/JMMserv

# Clean build files
clean:
	cd ./servidor && make clean
	cd ./log_server && make clean

