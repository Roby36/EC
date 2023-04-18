
E = ../executables
LIBS = -lm
MAKE = make

CFLAGS = -std=c++11
CC = g++

.PHONY: all strategies clean

all: strategies

strategies:
	$(CC) $(CFLAGS) $(LIBS) -DHOURLY -DS1 Strategies.cpp -o $E/S1hourly
	$(CC) $(CFLAGS) $(LIBS) -DHOURLY -DS2 Strategies.cpp -o $E/S2hourly
	$(CC) $(CFLAGS) $(LIBS) -DHOURLY -DS1P Strategies.cpp -o $E/S1Phourly
	$(CC) $(CFLAGS) $(LIBS) -DHOURLY -DS2P Strategies.cpp -o $E/S2Phourly

	$(CC) $(CFLAGS) $(LIBS) -DDAILY -DS1 Strategies.cpp -o $E/S1daily
	$(CC) $(CFLAGS) $(LIBS) -DDAILY -DS2 Strategies.cpp -o $E/S2daily
	$(CC) $(CFLAGS) $(LIBS) -DDAILY -DS1P Strategies.cpp -o $E/S1Pdaily
	$(CC) $(CFLAGS) $(LIBS) -DDAILY -DS2P Strategies.cpp -o $E/S2Pdaily

	$(CC) $(CFLAGS) $(LIBS) -DHOURLY -DSE Strategies.cpp -o $E/SEhourly


clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f core
	rm -f $E/S1hourly $E/S2hourly $E/S1Phourly $E/S2Phourly $E/S1daily $E/S2daily $E/S1Pdaily $E/S2Pdaily
	