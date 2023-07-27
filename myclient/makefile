
CXX				= g++
CXXFLAGS		= -pthread -Wall -Wno-switch -Wpedantic -Wno-unused-function -Wno-delete-non-abstract-non-virtual-dtor -std=c++11 -g -DDATEDBG # -DMEMDBG
ROOT_DIR		= ../twsapi/IBJts/source/cppclient
ANALYSIS_DIR	= ../analysis
BACKTESTING_DIR = ../backtesting
LIBS		    = -lboost_serialization #${ANALYSIS_DIR}/analysis.a ${BACKTESTING_DIR}/backtesting.a
BASE_SRC_DIR	= ${ROOT_DIR}/client
BOOST_DIR       = /usr/local/boost-1.81.0/include
BOOST_LIB       = /usr/local/boost-1.81.0/lib
INCLUDES		= -I${BASE_SRC_DIR} -I${ROOT_DIR} -I${ANALYSIS_DIR} -I${BACKTESTING_DIR} -I${BOOST_DIR} -L${BOOST_LIB}
SHARED_LIB_DIRS	= ${BASE_SRC_DIR}
SHARD_LIBS		= libTwsSocketClient.so
TARGET			= MClient

$(TARGET)Static: $(OBJS) $(ALIB)
	$(CXX) $(CXXFLAGS) $(OBJS) $(INCLUDES) $(BASE_SRC_DIR)/*.cpp $(ANALYSIS_DIR)/*.cpp $(BACKTESTING_DIR)/Trade.cpp $(BACKTESTING_DIR)/BackTester.cpp ./*.cpp $(BASE_SRC_DIR)/lib/libbid.a $(LIBS) -o$(TARGET)Static 

$(TARGET): $(OBJS) $(ALIB)
	$(CXX) $(CXXFLAGS) $(OBJS) $(INCLUDES) ./*.cpp $(BASE_SRC_DIR)/lib/libbid.so $(SHARED_LIB_DIRS)/$(SHARD_LIBS) $(LIBS) -o$(TARGET)

clean:
	rm -f $(TARGET) *.o
	rm -f ${ANALYSIS_DIR}/*.o
	rm -f $(TARGET)Static
	rm -f *.txt