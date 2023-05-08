
CXX				= g++
CXXFLAGS		= -pthread -Wall -Wno-switch -Wpedantic -Wno-unused-function -std=c++11
ROOT_DIR		= ../twsapi/IBJts/source/cppclient
ANALYSIS_DIR	= ../analysis
ALIB			= ${ANALYSIS_DIR}/analysis.a
BASE_SRC_DIR	= ${ROOT_DIR}/client
INCLUDES		= -I${BASE_SRC_DIR} -I${ROOT_DIR} -I${ANALYSIS_DIR}
SHARED_LIB_DIRS	= ${BASE_SRC_DIR}
SHARD_LIBS		= libTwsSocketClient.so
TARGET			= MClient

$(TARGET)Static: $(OBJS) $(ALIB)
	$(CXX) $(CXXFLAGS) $(OBJS) $(INCLUDES) $(BASE_SRC_DIR)/*.cpp ./*.cpp $(BASE_SRC_DIR)/lib/libbid.a $(ALIB) -o$(TARGET)Static

$(TARGET): $(OBJS) $(ALIB)
	$(CXX) $(CXXFLAGS) $(OBJS) $(INCLUDES) ./*.cpp $(BASE_SRC_DIR)/lib/libbid.so $(SHARED_LIB_DIRS)/$(SHARD_LIBS) $(ALIB) -o$(TARGET)

clean:
	rm -f $(TARGET) *.o
	rm -f ${ANALYSIS_DIR}/*.o
	rm -f $(TARGET)Static
	rm -f ./Instrument_log.txt ./MClient_log.txt