
A = ../analysis
G = ../graphs
T = ../testing
OBJS = testing.o
LIBS = -lm
LLIBS = $A/analysis.a
MAKE = make

CFLAGS = -I$A
CC = clang

.PHONY: all testing

all: testing

testing: $(OBJS)
	cd $A && $(MAKE)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) $(LLIBS) -o test
	

testing.o: testing.c $A/day.h $A/dataProcessing.h 

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f core
	rm -f test
	cd $A && $(MAKE) clean
	cd $G && rm -f dates.txt
	cd $G && rm -f closes.txt
	cd $G && rm -f volumes.txt
	cd $G && rm -f RSI.txt
	