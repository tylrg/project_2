CC=g++

CFLAGS=-Wall -Wextra -DDEBUG -g -std=c++14

all: cache-sim

cache-sim: cache-sim.o
	$(CC) $(CFLAGS) cache-sim.o -o cache-sim
cache-sim.o: cache-sim.cpp
	$(CC) $(CFLAGS) -c cache-sim.cpp -o cache-sim.o
clean:
	rm cache-sim *.o
