
A = ../analysis
OBJS = graph.o
LIBS = -lm
LLIBS = $A/analysis.a
MAKE = make

CFLAGS = -I$A
CC = clang

.PHONY: all graphdata graph clean

all: graph

graphdata: $(OBJS)
	cd $A && $(MAKE)
	mkdir data
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) $(LLIBS) -o graphdata

graph:
	$(MAKE) clean
	$(MAKE) graphdata && ./graphdata && /usr/local/bin/python3 /Users/roby/Desktop/EC/graphs/graph.py
	$(MAKE) clean

graph.o: graph.c $A/day.h $A/dataProcessing.h 

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f core
	rm -f graphdata
	rm -rf data
	cd $A && $(MAKE) clean
	
	
	