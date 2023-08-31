
#include "MClientTests.h"

void test_contract_details(MClient * client) {
    // client->reqContractDetails(101, MContractDetails::USBond().contract);
    // client->reqContractDetails(102, MContractDetails::USStockCFD().contract);
    // client->reqContractDetails(103, MContractDetails::IBMStockAtSmart().contract);
    // client->reqContractDetails(104, MContractDetails::AMZNStockAtSmart().contract);
    // client->reqContractDetails(105, MContractDetails::NDAQIndex().contract);
    client->reqContractDetails(106, MContractDetails::NDAQStk().contract);
}

void durationStr_test(MClient * client) {
    client->add_Instrument( "5 secs", MContractDetails::DAXInd(), MContractDetails::DAXFut(),    
                            Instrument::ReqIds(101, 201, 301, 401, 501), "../instruments_log/Dax_short.txt");
    client->update_bars(1024, true);
    std::this_thread::sleep_for(std::chrono::seconds(5)); // SLEEP BEFORE NEXT REQUEST!
    client->update_bars(128, false);
}

void run_backtests(MClient * client) {
    
    init_strategies(client);

/* Esegui i backtesting su ogni strategia */
    client->set_trading_state(BACKTESTING);
    // aggiorna dati contratti 
    client->update_contracts();
    // Determina il numero di barre da includere nei backtest 
    client->update_bars(num_bars, true);
    // stampa risultati backtests 
    client->print_indicators(outputDir);
    client->print_bars(outputDir);
    client->print_PL_data(outputDir);
    client->print_backtest_results();

    del_strategies();
}


void run_livetrades(MClient * client) {
    
    init_strategies(client);

    client->update_contracts(); 
    // To test live trading on bar retrieval data:
    client->set_trading_state(LIVE); 
    client->update_bars(num_bars, true);
    // give time for executions to come in
    std::this_thread::sleep_for(std::chrono::seconds(15));

    del_strategies();
}

void trading_loop(MClient * client, int loop_dur) {
    
    init_strategies(client);

    // aggiorna dati contratti 
    client->update_contracts();
    // Determina il numero di barre da includere nei backtest 
    client->update_bars(num_bars, true);
    
    /* Subscribe to realTimeBars for live trading */
    client->set_trading_state(LIVE); // SET LIVE TRADING STATE AFTER ADDING STRATEGY AND AFTER INITIALIZING BARS!
    client->reqRealTimeBars(instr[0]->m_reqIds.realTimeBars, instr[0]->dataContract.contract, -1, "TRADES", 1);
#ifndef SS_TEST
    client->reqRealTimeBars(instr[1]->m_reqIds.realTimeBars, instr[1]->dataContract.contract, -1, "TRADES", 1);
    client->reqRealTimeBars(instr[2]->m_reqIds.realTimeBars, instr[2]->dataContract.contract, -1, "TRADES", 1);
#endif

    /* Enter loop, trying to update bars every 5 seconds */
    int dur_elapsed = 0;
    const int update_frequency = 5; // = barSize (seconds)
    printf("Entering trading loop...\n");
    while (dur_elapsed < loop_dur) {
        /** 20S = invalid duration */
        client->update_bars(12, false);
        std::this_thread::sleep_for(std::chrono::seconds(update_frequency));
        dur_elapsed += update_frequency;
    }

    printf("Exited trading loop. Printing logs...\n");
    /* Print all logs, as if we were in backtesting mode */
    client->print_indicators(outputDir);
    client->print_bars(outputDir);
    client->print_PL_data(outputDir);
    client->print_backtest_results();

    del_strategies();
}

void init_strategies(MClient * client)
{
    /* Initialize GLOBAL instruments, indicators, and strategies */
    inst_id[0] = client->add_Instrument(barSize_1, dataContract_1, orderContract_1,
                                    Instrument::ReqIds(610, 620, 630, 640, 650), "../instruments_log/backtest_instr1.txt"); 
    instr[0] = client->get_Instrument(inst_id[0]);
#ifndef SS_TEST
    inst_id[1] = client->add_Instrument(barSize_2, dataContract_2, orderContract_2,
                                    Instrument::ReqIds(710, 720, 730, 740, 750), "../instruments_log/backtest_instr2.txt"); 
    instr[1] = client->get_Instrument(inst_id[1]);
    inst_id[2] = client->add_Instrument(barSize_3, dataContract_3, orderContract_3,
                                    Instrument::ReqIds(810, 820, 830, 840, 850), "../instruments_log/backtest_instr3.txt"); 
    instr[2] = client->get_Instrument(inst_id[2]);
#endif
/* Inizializza gli indicatori per i vari mercati  */  
    ADD_INDICATORS(instr[0], localMin[0], localMax[0], Rsi[0], bollBands[0], divergence[0], longDivergence[0]);
#ifndef SS_TEST
    ADD_INDICATORS(instr[1], localMin[1], localMax[1], Rsi[1], bollBands[1], divergence[1], longDivergence[1]);
    ADD_INDICATORS(instr[2], localMin[2], localMax[2], Rsi[2], bollBands[2], divergence[2], longDivergence[2]);
#endif
/* Utilizzando gli strumenti, le condizioni, e gli indicatori definiti qua sopra, inizializziamo strategie */
    INIT_STRATEGY(strat[0], strategy_code_1, instr[0], localMin[0], localMax[0], Rsi[0], bollBands[0], divergence[0], longDivergence[0]);
#ifndef SS_TEST
    INIT_STRATEGY(strat[1], strategy_code_2, instr[1], localMin[1], localMax[1], Rsi[1], bollBands[1], divergence[1], longDivergence[1]);
    INIT_STRATEGY(strat[2], strategy_code_3, instr[2], localMin[2], localMax[2], Rsi[2], bollBands[2], divergence[2], longDivergence[2]);
#endif
/* Aggiungiamo strategie */
    client->add_Strategy(inst_id[0], strat[0]);
#ifndef SS_TEST
    client->add_Strategy(inst_id[1], strat[1]);
    client->add_Strategy(inst_id[2], strat[2]);
#endif
}

void del_strategies() {
    /* Elimina strategie */
    delete (strat[0]);
#ifndef SS_TEST
    delete (strat[1]);
    delete (strat[2]);
#endif
}

