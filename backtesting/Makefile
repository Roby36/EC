
E = ../executables
LIBS = -lm
MAKE = make

CFLAGS = -std=c++11
CC = g++
SOURCE = Strategies.cpp ../analysis/Bars.cpp ../analysis/Bar.cpp

.PHONY: all strategies clean

all: strategies

strategies:
	$(CC) $(CFLAGS) $(LIBS) -DHOURLY -DS1  $(SOURCE) -o $E/S1hourly
	$(CC) $(CFLAGS) $(LIBS) -DHOURLY -DS2  $(SOURCE) -o $E/S2hourly
	$(CC) $(CFLAGS) $(LIBS) -DHOURLY -DS1P  $(SOURCE) -o $E/S1Phourly
	$(CC) $(CFLAGS) $(LIBS) -DHOURLY -DS2P  $(SOURCE) -o $E/S2Phourly

	$(CC) $(CFLAGS) $(LIBS) -DDAILY -DS1  $(SOURCE) -o $E/S1daily
	$(CC) $(CFLAGS) $(LIBS) -DDAILY -DS2  $(SOURCE) -o $E/S2daily
	$(CC) $(CFLAGS) $(LIBS) -DDAILY -DS1P  $(SOURCE) -o $E/S1Pdaily
	$(CC) $(CFLAGS) $(LIBS) -DDAILY -DS2P  $(SOURCE) -o $E/S2Pdaily

	$(CC) $(CFLAGS) $(LIBS) -DHOURLY -DSE  $(SOURCE) -o $E/SEhourly
	$(CC) $(CFLAGS) $(LIBS) -DHOURLY -DS4  $(SOURCE) -o $E/S4hourly


clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f core
	rm -f $E/S1hourly $E/S2hourly $E/S1Phourly $E/S2Phourly $E/S1daily $E/S2daily $E/S1Pdaily $E/S2Pdaily $E/SEhourly  $E/S4hourly
	