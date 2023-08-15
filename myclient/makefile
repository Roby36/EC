
CXX				= g++
CXXFLAGS		= -pthread -Wall -Wno-switch -Wpedantic -Wno-unused-function -Wno-delete-non-abstract-non-virtual-dtor -std=c++11 -g -DDATEDBG -DINDDBG  # -DMEMDBG 

ROOT_DIR		= ../twsapi/IBJts/source/cppclient
ANALYSIS_DIR	= ../analysis
BACKTESTING_DIR = ../backtesting
CONTRACTS_DIR   = ../contracts
TESTING_DIR     = ../testing
UTILS_DIR       = ../utils
LIBS		    = -lboost_serialization #${ANALYSIS_DIR}/analysis.a ${BACKTESTING_DIR}/backtesting.a
BASE_SRC_DIR	= ${ROOT_DIR}/client
BOOST_DIR       = /usr/local/boost-1.81.0/include
BOOST_LIB       = /usr/local/boost-1.81.0/lib
INCLUDES		= -I${BASE_SRC_DIR} -I${ROOT_DIR} -I${ANALYSIS_DIR} -I${BACKTESTING_DIR} -I${CONTRACTS_DIR} -I${TESTING_DIR} -I${UTILS_DIR} -I${BOOST_DIR} -L${BOOST_LIB}
SHARED_LIB_DIRS	= ${BASE_SRC_DIR}
SHARD_LIBS		= libTwsSocketClient.so
TARGET			= MClient

$(TARGET)Static: $(OBJS) $(ALIB)
	$(CXX) $(CXXFLAGS) $(OBJS) $(INCLUDES) $(BASE_SRC_DIR)/*.cpp $(ANALYSIS_DIR)/*.cpp $(BACKTESTING_DIR)/Trade.cpp $(BACKTESTING_DIR)/BackTester.cpp ./*.cpp $(CONTRACTS_DIR)/*.cpp  $(TESTING_DIR)/*.cpp $(UTILS_DIR)/*.cpp $(BASE_SRC_DIR)/lib/libbid.a $(LIBS) -o$(TARGET)Static 

$(TARGET): $(OBJS) $(ALIB)
	$(CXX) $(CXXFLAGS) $(OBJS) $(INCLUDES) ./*.cpp $(BASE_SRC_DIR)/lib/libbid.so $(SHARED_LIB_DIRS)/$(SHARD_LIBS) $(LIBS) -o$(TARGET)

clean:
	rm -f $(TARGET) *.o
	rm -f ${ANALYSIS_DIR}/*.o
	rm -f $(TARGET)Static
	rm -f ./*.txt
	rm -rf *.dSYM
	rm -f ../backtesting/logs/*.txt
	rm -f ../backtesting/reports/*.txt
	rm -f ../graphs/data/*.txt
	rm -f ../indicators_log/*.txt
	rm -f ../instruments_log/*.txt
	rm -f ../strategies_log/*.txt
	


