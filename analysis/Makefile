
OBJS = day.o dataProcessing.o
LIB = analysis.a

.PHONY: all

all: $(LIB)
# Build $(LIB) by archiving object files
$(LIB): $(OBJS)
	ar cr $(LIB) $(OBJS)

day.o: day.h
dataProcessing.o: dataProcessing.h

clean:
	rm -f core
	rm -f $(LIB) *~ *.o