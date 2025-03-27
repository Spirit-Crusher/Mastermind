
# Default all
all: serv log apl

################################## BUILD ##################################
# Build server
serv: 
#	$(CC) $^ -o $@
	cd ./servidor && make

# Build log
log: 
#	$(CC) $^ -o $@
	cd ./log_server && make

# Build client
apl: 
#	$(CC) $^ -o $@
	cd ./client && make

################################## RUN ##################################
# run server
run_s:

	cd ./servidor && make && ./JMMserv

# run log server
run_l:
	cd ./log_server && make && ./JMMlog

# run client
run_c:
	cd ./client && make && ./JMMapl

################################## CLEAN ##################################
# Clean build files
clean:
	cd ./servidor && make clean
	cd ./log_server && make clean
	cd ./client && make clean

