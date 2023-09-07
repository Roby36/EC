
#include "MClient.h"
#include "CommonMacros.h"
#include "../testing/MClientTests.h"

int main() {

    // Attempt connection to localhost port 7497 with clientId of 0
    printf( "Start of MClient Test\n");

/** IMPORTANT: Trade data must always be initialized from scratch when backtesting !! 
 *  TODO:   Handle this case to fix this backtesting limitation
*/
    bool init_trade_data;
#ifdef BACKTEST
    init_trade_data = true;
#else 
    init_trade_data = true;    // set trading data initialization manually here when not backtesting
#endif

    MClient * client = new MClient("./MClient_log.txt", 17001, init_trade_data);

    client->connect( "", 7497);

    // GIVE TIME TO ESTABLISH CONNECTION
    std::this_thread::sleep_for(std::chrono::seconds(5));

	while (!client->isConnected()) {
        client->connect( "", 7497);
		printf( "Sleeping %u seconds before next attempt\n", 5);
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}

    // Give more time
    std::this_thread::sleep_for(std::chrono::seconds(5));

#ifdef TEST_CONTRACT_DETAILS
    test_contract_details(client);
#endif

#ifdef DURSTRTEST
    durationStr_test(client);
#endif

    /** BACKTESTING: */
#ifdef BACKTEST
    run_backtests(client);
#endif

    /** TRADING: */
#ifdef LIVETRADE
    run_livetrades(client);
   // Was trading data stored correctly?
    client->testSerFile();
#endif

#ifdef TRADINGLOOP
    trading_loop(client, 14400);
    client->testSerFile();
#endif

// Balance orders when too many are open in a given direction
#ifdef ORDER_BALANCING
    client->placeOrders(inst_id[0],  MOrders::MarketOrder("BUY", doubleToDecimal(1.0)), 30);
    #ifndef SS_TEST
    client->placeOrders(inst_id[1],  MOrders::MarketOrder("BUY", doubleToDecimal(1.0)), 30);
    client->placeOrders(inst_id[2],  MOrders::MarketOrder("BUY", doubleToDecimal(1.0)), 30);
    #endif
#endif

    // Cancel all placed orders
#ifdef GLOBALCANCEL
    client->reqGlobalCancel();
#endif

    // clean up
    delete(client);
    printf( "End of MClient Test\n");
    return 0;

}
