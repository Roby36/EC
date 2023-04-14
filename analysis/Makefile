
E = ../executables
LIBS = -lm
MAKE = make

CFLAGS = -std=c++11
CC = g++

.PHONY: all strategies clean

all: strategies

strategies:
	$(CC) $(CFLAGS) $(LIBS) -DS1 Strategies.cpp -o $E/S1hourly
	$(CC) $(CFLAGS) $(LIBS) -DS2 Strategies.cpp -o $E/S2hourly
	$(CC) $(CFLAGS) $(LIBS) -DDAILY -DS1 Strategies.cpp -o $E/S1daily
	$(CC) $(CFLAGS) $(LIBS) -DDAILY -DS2 Strategies.cpp -o $E/S2daily


clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f core
	rm -f $E/S1hourly $E/S2hourly $E/S1daily $E/S2daily
	