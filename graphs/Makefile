
A = ../analysis
OBJS = graph.o
LIBS = -lm
LLIBS = $A/analysis.a
MAKE = make

MACD = -DMACDG
CFLAGS = -I$A $(MACD)
CC = clang

.PHONY: all graphdata graphRSI graphMACD clean

all: graphRSI

graphdata: $(OBJS)
	cd $A && $(MAKE)
	mkdir data
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) $(LLIBS) -o graphdata

graphRSI:
	$(MAKE) graphdata && ./graphdata && /usr/local/bin/python3 /Users/roby/Desktop/EC/graphs/graphRSI.py
	$(MAKE) clean

graphMACD:
	$(MAKE) graphdata && ./graphdata && /usr/local/bin/python3 /Users/roby/Desktop/EC/graphs/graphMACD.py
	$(MAKE) clean


graph.o: graph.c $A/day.h $A/dataProcessing.h 

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f core
	rm -f graphdata
	rm -rf data
	cd $A && $(MAKE) clean
	
	
	