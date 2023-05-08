
CXX			= g++
CXXFLAGS	= -Wall -Wno-switch -Wpedantic -Wno-unused-function -std=c++11
LIB         = analysis.a
IST         = IsetTest
OBJS 		= Bar.o Bars.o Indicator.o Indicators.o IndicatorSet.o

.PHONY: all clean

all: $(LIB) $(IST)

$(LIB): $(OBJS)
	ar cr $(LIB) $^

# Make all object files and archive them in library
Bar.o: Bar.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

Bars.o: Bars.cpp Bar.h 
	$(CXX) $(CXXFLAGS) -c $< -o $@

Indicator.o: Indicator.cpp Bars.h 
	$(CXX) $(CXXFLAGS) -c $< -o $@

Indicators.o: Indicators.cpp IndicatorBar.cpp Indicator.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

IndicatorSet.o: IndicatorSet.cpp Indicators.cpp Bars.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Make IST target
$(IST): IndicatorSet.cpp Indicator.cpp Bars.cpp Bar.cpp
	$(CXX)  -DINDTEST $(CXXFLAGS) $^ -o $@

clean:
	rm -f *.o
	rm -f $(IST)