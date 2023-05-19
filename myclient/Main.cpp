
#include "MClient.h"
#include "MContractDetails.h"
#include "MOrders.h"

#include <stdio.h>
#include <stdlib.h>
#include <chrono>

int main() {

    // Attempt connection to localhost port 7497 with clientId of 0
    printf( "Start of MClient Test\n");

    MClient* client = new MClient();

    client->connect( "", 7497);

    // GIVE TIME TO ESTABLISH CONNECTION
    std::this_thread::sleep_for(std::chrono::seconds(5));

	while (!client->isConnected()) {
        client->connect( "", 7497);
		printf( "Sleeping %u seconds before next attempt\n", 5);
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}

    // Set up an instrument at low time-interval
    client->add_Instrument( "1 min", MContractDetails::DAXInd(), MContractDetails::DAXFut(),    
                            Instrument::reqIds(101, 201, 301, 401, 501), "Dax.txt");
    // Set up crypto-instrument to test bars update
    client->add_Instrument( "1 min", MContractDetails::CryptoContract(), MContractDetails::CryptoContract(),    
                            Instrument::reqIds(1101, 1201, 1301, 1401, 1501), "Btc.txt");
    // Update contract details and initialize bars data for the instrument
    client->update_contracts();  
    client->initialize_bars("1 D");

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
    
    
    /**** ORDER TEST ***/
    
    // Place MarketOrder on first Instrument, specifying information regarding strategy
    
    
    int orderId; 
    // orderId = client->placeOrder( 0, MOrders::MarketOrder("SELL", doubleToDecimal(1.0), "S1"));
    // orderId = client->placeOrder( 0, MOrders::MarketOrder("SELL", doubleToDecimal(1.0), "S1"));
    // orderId = client->placeOrder( 0, MOrders::MarketOrder("SELL", doubleToDecimal(1.0), "S1"));
    
    // orderId = client->placeOrder( 1, MOrders::MarketOrder("SELL", doubleToDecimal(1.0), "S1"));
    // orderId = client->placeOrder( 1, MOrders::MarketOrder("SELL", doubleToDecimal(1.0), "S1"));
    // orderId = client->placeOrder( 1, MOrders::MarketOrder("SELL", doubleToDecimal(1.0), "S1"));


    // Give order time to be submitted
    // std::this_thread::sleep_for(std::chrono::seconds(5));
    // Cancel order (DOES NOT CLOSE TRADE)
    // client->cancelOrder(orderId);

    // Request all open orders
    client->reqAllOpenOrders();


    // Print out bar data from the first instrument
    client->get_Instrument(1)->bars->printBars();
    
    // Clean up
    delete(client);
    printf( "End of MClient Test\n");
    return 0;
}