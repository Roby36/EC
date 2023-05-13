
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

	for (;;) {

		client->connect( "", 7497);

        // GIVE TIME TO ESTABLISH CONNECTION
        std::this_thread::sleep_for(std::chrono::seconds(5));
		
		if( client->isConnected()) {
			break;
		}
		printf( "Sleeping %u seconds before next attempt\n", 5);
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}

    // Set up an instrument at low time-interval
    client->add_Instrument( "1 min", MContractDetails::DAXInd(), MContractDetails::DAXFut(),    
                            Instrument::reqIds(101, 201, 301, 401, 501));
    // Update contract details and initialize bars data for the instrument
    client->update_contracts();  
    client->initialize_bars("1 D");

    // Update bars a few times
    std::this_thread::sleep_for(std::chrono::seconds(30));
    client->update_bars();
    std::this_thread::sleep_for(std::chrono::seconds(30));
    client->update_bars();
    std::this_thread::sleep_for(std::chrono::seconds(60));
    client->update_bars();
    std::this_thread::sleep_for(std::chrono::seconds(15));
    client->update_bars();
    std::this_thread::sleep_for(std::chrono::seconds(15));
    client->update_bars();


    /**** ORDER TEST ***/
    /*
        // Place MarketOrder on first Instrument, specifying information regarding strategy
        int orderId = client->placeOrder( 0, MOrders::MarketOrder("SELL", doubleToDecimal(1.0), "S1"));
        // Give order time to be submitted
        std::this_thread::sleep_for(std::chrono::seconds(5));
        // Cancel order (DOES NOT CLOSE TRADE)
        client->cancelOrder(orderId);
    */

    // Print out bar data from the first instrument
    client->get_Instrument(0)->bars->printBars();
    
    // Clean up
    delete(client);
    printf( "End of MClient Test\n");
    return 0;
}