
#include "MClientTests.h"

void test_contract_details(MClient * client) {
    client->reqContractDetails(101, MContractDetails::USBond().contract);
    client->reqContractDetails(102, MContractDetails::USStockCFD().contract);
    client->reqContractDetails(103, MContractDetails::IBMStockAtSmart().contract);
    client->reqContractDetails(104, MContractDetails::AMZNStockAtSmart().contract);
}

void run_livetrades(MClient * client, int loop_dur) {
    /** TODO: Add instrument & strategies from Backtest.cpp to repeat Backtest trades but with real orders! */
    int dax_short_id = client->add_Instrument( "5 secs", MContractDetails::DAXInd(), MContractDetails::DAXFut(),    
                                               Instrument::ReqIds(101, 201, 301, 401, 501), "../instruments_log/Dax_short.txt");
    Instrument * dax_short_instr = client->get_Instrument(dax_short_id);
    EntryConditions S1_entry_conditions  [] = {DENIED_DIVERGENCE, ENTRY_CONDITIONS_END}; 
    EntryConditions S2_entry_conditions  [] = {DOUBLE_DIVERGENCE, ENTRY_CONDITIONS_END};
    ExitConditions  S1a2_exit_conditions []  = {OPPOSITE_DIVERGENCE,                     
                                                BOLLINGER_CROSSING, 
                                                NEGATIVE_TRADE_EXPIRATION, 
                                                STOP_LOSS_TAKE_PROFIT, 
                                                EXIT_CONDITIONS_END};
    /* Set divergence periods */
    const int min_div_period = 2;
    const int max_div_period = 14;
    /* S1 indicators */
    Indicators::LocalMin *       S1_short_LocalMin       = new Indicators::LocalMin      (dax_short_instr->bars); 
    Indicators::LocalMax *       S1_short_LocalMax       = new Indicators::LocalMax      (dax_short_instr->bars);
    Indicators::RSI      *       S1_short_RSI            = new Indicators::RSI           (dax_short_instr->bars);
    Indicators::BollingerBands * S1_short_BollingerBands = new Indicators::BollingerBands(dax_short_instr->bars, 2.0, 2.0, 20); // NOTA: S1 utilizza le Bollinger a 2.0
    Indicators::Divergence *     S1_short_Divergence     = new Indicators::Divergence    (dax_short_instr->bars, S1_short_LocalMax, S1_short_LocalMin, S1_short_RSI, min_div_period, max_div_period);
    Indicators::LongDivergence * S1_short_LongDivergence = new Indicators::LongDivergence(dax_short_instr->bars, S1_short_LocalMax, S1_short_LocalMin, S1_short_RSI, min_div_period, max_div_period);
    /* S2 indicators */
    Indicators::LocalMin *       S2_short_LocalMin       = new Indicators::LocalMin      (dax_short_instr->bars); 
    Indicators::LocalMax *       S2_short_LocalMax       = new Indicators::LocalMax      (dax_short_instr->bars);
    Indicators::RSI      *       S2_short_RSI            = new Indicators::RSI           (dax_short_instr->bars);
    Indicators::BollingerBands * S2_short_BollingerBands = new Indicators::BollingerBands(dax_short_instr->bars, 2.5, 2.5, 20); // NOTA: S2 utilizza le Bollinger a 2.5
    Indicators::Divergence *     S2_short_Divergence     = new Indicators::Divergence    (dax_short_instr->bars, S2_short_LocalMax, S2_short_LocalMin, S2_short_RSI, min_div_period, max_div_period);
    Indicators::LongDivergence * S2_short_LongDivergence = new Indicators::LongDivergence(dax_short_instr->bars, S2_short_LocalMax, S2_short_LocalMin, S2_short_RSI, min_div_period, max_div_period);
    /* Strategies */
    Strategy * S1_short = new Strategy(dax_short_instr, client->m_tradeData,
                                    2.5, 2.5, 18, /* Settare qua stop loss, take profit, e massime barre prima della chiusura in negativo*/
                                    "S1_short", S1_entry_conditions, S1a2_exit_conditions,
                                    S1_short_LocalMin, S1_short_LocalMax, S1_short_RSI, S1_short_BollingerBands, S1_short_Divergence, S1_short_LongDivergence
                                    );
    Strategy * S2_short = new Strategy(dax_short_instr, client->m_tradeData, 
                                    3.5, 3.5, 63, /* Settare qua stop loss, take profit, e massime barre prima della chiusura in negativo*/
                                    "S2_short", S2_entry_conditions, S1a2_exit_conditions,
                                    S2_short_LocalMin, S2_short_LocalMax, S2_short_RSI, S2_short_BollingerBands, S2_short_Divergence, S2_short_LongDivergence
                                    );
    client->add_Strategy(dax_short_id, S1_short);
    client->add_Strategy(dax_short_id, S2_short);
    client->update_contracts(); 
    // Balance orders when too many are open in a given direction
#ifdef ORDER_BALANCING
    client->placeOrders(dax_short_id,  MOrders::MarketOrder("BUY", doubleToDecimal(1.0)), 30);
#else
    // To test live trading on bar retrieval data:
    client->set_trading_state(LIVE); 
    client->update_bars(4096, true);
    // give time for executions to come in
    std::this_thread::sleep_for(std::chrono::seconds(30)); 
    client->set_trading_state(LIVE); // SET LIVE TRADING STATE AFTER ADDING STRATEGY AND AFTER INITIALIZING BARS!
    client->reqRealTimeBars(dax_short_instr->m_reqIds.realTimeBars,
                            dax_short_instr->dataContract.contract,
                            -1, // "currently ignored", realtimebars at 5 second-intervals
                            "TRADES",
                            1);
    // Enter loop, trying to update bars every 5 seconds
    int dur_elapsed = 0;
    const int update_frequency = 5;
    const int max_dur = loop_dur;
    while (dur_elapsed < max_dur) {
        /** 20S = invalid duration */
        client->update_bars(12, false);
        std::this_thread::sleep_for(std::chrono::seconds(update_frequency));
        dur_elapsed += update_frequency;
    }
#endif
    delete(S1_short);
    delete(S2_short);
}

void durationStr_test(MClient * client) {
    client->add_Instrument( "5 secs", MContractDetails::DAXInd(), MContractDetails::DAXFut(),    
                            Instrument::ReqIds(101, 201, 301, 401, 501), "../instruments_log/Dax_short.txt");
    client->update_bars(1024, true);
    std::this_thread::sleep_for(std::chrono::seconds(5)); // SLEEP BEFORE NEXT REQUEST!
    client->update_bars(128, false);
}
