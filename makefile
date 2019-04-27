CC=g++

CFLAGS=-Wall -DDEBUG -g -std=c++14

all: cache-sim


predictors: cache-sim.o
	$(CC) $(CFLAGS) cache-sim.o -o cache-sim
predictors.o: cache-sim.h cache-sim.cpp
	$(CC) $(CFLAGS) -c cache-sim.cpp -o cache-sim.o

clean:
	rm cache-sim *.o