
CXX			= g++
CXXFLAGS	= -Wall -Wno-switch -Wpedantic -Wno-unused-function -std=c++11
LIB         = analysis.a
# LIBS		= -lboost_serialization
# BOOST_DIR   = /usr/local/boost-1.81.0/include
# BOOST_LIB   = /usr/local/boost-1.81.0/lib
# INCLUDES    = -I${BOOST_DIR} -L${BOOST_LIB}
IST         = IsetTest
OBJS 		= Bar.o Bars.o Indicator.o Indicators.o IndicatorSet.o

.PHONY: all clean

all: $(LIB) $(IST)

$(LIB): $(OBJS)
	ar cr $(LIB) $^

# Make all object files and archive them in library
Bar.o: Bar.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< $(LIBS) -o $@ 

Bars.o: Bars.cpp 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< $(LIBS) -o $@

Indicator.o: Indicator.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< $(LIBS) -o $@

IndicatorBar.o: IndicatorBar.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< $(LIBS) -o $@

Indicators.o: Indicators.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< $(LIBS) -o $@

IndicatorSet.o: IndicatorSet.cpp 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< $(LIBS) -o $@

# Make IST target
$(IST): IndicatorSet.cpp Indicator.cpp Bars.cpp Bar.cpp
	$(CXX)  -DINDTEST $(CXXFLAGS) $(INCLUDES) $^ $(LIBS) -o $@

clean:
	rm -f *.o
	rm -f $(IST)