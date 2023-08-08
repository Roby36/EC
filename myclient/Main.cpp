
#include "MClient.h"
#include "CommonMacros.h"
#include <chrono>
#include "MClientTests.h"

int main() {

    // Attempt connection to localhost port 7497 with clientId of 0
    printf( "Start of MClient Test\n");

    MClient* client = new MClient("./MClient_log.txt", 17001, true);

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

    /** INSTRUMENTS: */
    // Set up an instrument at low time interval for testing
/*
    int inst1_id = client->add_Instrument( "5 secs", MContractDetails::CryptoContract(), MContractDetails::CryptoContract(),    
                                            Instrument::ReqIds(1101, 1201, 1301, 1401, 1501), "Btc.txt");
    int inst2_id = client->add_Instrument( "5 secs", MContractDetails::EurGbpFx(), MContractDetails::EurGbpFx(),    
                    Instrument::ReqIds(2101, 2201, 2301, 2401, 2501), "EurGbp.txt");
*/

    /** TRADING: */
#ifdef LIVETRADE
   run_livetrades(client);
#endif

    // clean up
    delete(client);
    printf( "End of MClient Test\n");
    return 0;

    // VARIOUS TESTS:
    // Update bars a few times
    /*
    std::this_thread::sleep_for(std::chrono::seconds(60));
    client->update_bars("TRADES", 1, 2);
    std::this_thread::sleep_for(std::chrono::seconds(60));
    client->update_bars("TRADES", 1, 2);
    std::this_thread::sleep_for(std::chrono::seconds(60));
    client->update_bars("TRADES", 1, 2);
    std::this_thread::sleep_for(std::chrono::seconds(60));
    client->update_bars("TRADES", 1, 2);
    */
    // Print out bar data from the first instrument
    // client->get_Instrument(1)->bars->printBars();
    
    /* TRADES TEST */
    /*
    client->openTrade("S1", 0, 
                    MOrders::MarketOrder("BUY",  doubleToDecimal(1.0)),
                    MOrders::MarketOrder("SELL", doubleToDecimal(1.0)));
    client->closeTrade(0);
    // Was trade information stored properly ??
    client->testSerFile();
    */

}