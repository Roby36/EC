
#include "MClientTests.h"

void test_contract_details(MClient * client) {
    client->reqContractDetails(101, MContractDetails::USBond().contract);
    client->reqContractDetails(102, MContractDetails::USStockCFD().contract);
    client->reqContractDetails(103, MContractDetails::IBMStockAtSmart().contract);
    client->reqContractDetails(104, MContractDetails::AMZNStockAtSmart().contract);
}

void run_backtests(MClient * client) {
/* Aggiungi strumento DAX hourly */
    int dax_hourly_id = client->add_Instrument( "1 hour", MContractDetails::DAXInd(), MContractDetails::DAXFut(),    
                                    Instrument::ReqIds(3101, 3201, 3301, 3401, 3501), "../instruments_log/Dax_hourly.txt");
/* Aggiungi strumento DAX daily */
    int dax_daily_id = client->add_Instrument( "1 day", MContractDetails::DAXInd(), MContractDetails::DAXFut(),    
                                    Instrument::ReqIds(4101, 4201, 4301, 4401, 4501), "../instruments_log/Dax_daily.txt");
/* Definisci le condizioni di entrata e uscita (mettendo sempre "ENTRY_CONDITIONS_END" alla fine della lista)*/
    EntryConditions S1_entry_conditions  [] = {DENIED_DIVERGENCE, ENTRY_CONDITIONS_END}; // entrata per S1
    EntryConditions S2_entry_conditions  [] = {DOUBLE_DIVERGENCE, ENTRY_CONDITIONS_END}; // entrata per S2
    ExitConditions  S1a2_exit_conditions []  = {OPPOSITE_DIVERGENCE,                     // uscita per S1 e S2
                                                BOLLINGER_CROSSING, 
                                                NEGATIVE_TRADE_EXPIRATION, 
                                                STOP_LOSS_TAKE_PROFIT, 
                                                EXIT_CONDITIONS_END};
/* Inizializza gli indicatori per ogni strategia, e per ogni grandezza di barre (4 blocchi in totale)
   Qua ad esempio decidiamo l'ampiezza delle Bollinger ed altre variabili degli indicatori,
   che poi verranno prese in considerazione per dare i vari segnali di entrata e uscita */
    Instrument * dax_hourly_instr = client->get_Instrument(dax_hourly_id);
    Instrument * dax_daily_instr  = client->get_Instrument(dax_daily_id);
    /* Setta periodi divergenze */
    const int min_div_period = 1;  /* minimo peiodo tra i due massimi/minimi di una divergenza */
    const int max_div_period = 14; /* massimo peiodo tra i due massimi/minimi di una divergenza */
    const int max_neg_period = 28; /* massimo peiodo (in barre) per la negazione */
    /* Indicatori per S1 hourly */
    Indicators::LocalMin *       S1_hourly_LocalMin       = new Indicators::LocalMin      (dax_hourly_instr->bars); 
    Indicators::LocalMax *       S1_hourly_LocalMax       = new Indicators::LocalMax      (dax_hourly_instr->bars);
    Indicators::RSI      *       S1_hourly_RSI            = new Indicators::RSI           (dax_hourly_instr->bars);
    Indicators::BollingerBands * S1_hourly_BollingerBands = new Indicators::BollingerBands(dax_hourly_instr->bars, 2.0, 2.0, 20); // NOTA: S1 utilizza le Bollinger a 2.0
        //!! Separate indicators because these are the only indicators changed by each strategy (by marking denied divergences!)
    Indicators::Divergence *     S1_hourly_Divergence0     = new Indicators::Divergence    (dax_hourly_instr->bars, S1_hourly_LocalMax, S1_hourly_LocalMin, S1_hourly_RSI, min_div_period, max_div_period);
    Indicators::LongDivergence * S1_hourly_LongDivergence0 = new Indicators::LongDivergence(dax_hourly_instr->bars, S1_hourly_LocalMax, S1_hourly_LocalMin, S1_hourly_RSI, min_div_period, max_div_period);
    Indicators::Divergence *     S1_hourly_Divergence1     = new Indicators::Divergence    (dax_hourly_instr->bars, S1_hourly_LocalMax, S1_hourly_LocalMin, S1_hourly_RSI, min_div_period, max_div_period);
    Indicators::LongDivergence * S1_hourly_LongDivergence1 = new Indicators::LongDivergence(dax_hourly_instr->bars, S1_hourly_LocalMax, S1_hourly_LocalMin, S1_hourly_RSI, min_div_period, max_div_period);
    /* Indicatori per S2 hourly */
    Indicators::LocalMin *       S2_hourly_LocalMin       = new Indicators::LocalMin      (dax_hourly_instr->bars); 
    Indicators::LocalMax *       S2_hourly_LocalMax       = new Indicators::LocalMax      (dax_hourly_instr->bars);
    Indicators::RSI      *       S2_hourly_RSI            = new Indicators::RSI           (dax_hourly_instr->bars);
    Indicators::BollingerBands * S2_hourly_BollingerBands = new Indicators::BollingerBands(dax_hourly_instr->bars, 2.5, 2.5, 20); // NOTA: S2 utilizza le Bollinger a 2.5
    Indicators::Divergence *     S2_hourly_Divergence     = new Indicators::Divergence    (dax_hourly_instr->bars, S2_hourly_LocalMax, S2_hourly_LocalMin, S2_hourly_RSI, min_div_period, max_div_period);
    Indicators::LongDivergence * S2_hourly_LongDivergence = new Indicators::LongDivergence(dax_hourly_instr->bars, S2_hourly_LocalMax, S2_hourly_LocalMin, S2_hourly_RSI, min_div_period, max_div_period);
    /* Indicatori per S1 daily*/
    Indicators::LocalMin *       S1_daily_LocalMin       = new Indicators::LocalMin      (dax_daily_instr->bars); 
    Indicators::LocalMax *       S1_daily_LocalMax       = new Indicators::LocalMax      (dax_daily_instr->bars);
    Indicators::RSI      *       S1_daily_RSI            = new Indicators::RSI           (dax_daily_instr->bars);
    Indicators::BollingerBands * S1_daily_BollingerBands = new Indicators::BollingerBands(dax_daily_instr->bars, 2.0, 2.0, 20); // NOTA: S1 utilizza le Bollinger a 2.0
    Indicators::Divergence *     S1_daily_Divergence     = new Indicators::Divergence    (dax_daily_instr->bars, S1_daily_LocalMax, S1_daily_LocalMin, S1_daily_RSI, min_div_period, max_div_period);
    Indicators::LongDivergence * S1_daily_LongDivergence = new Indicators::LongDivergence(dax_daily_instr->bars, S1_daily_LocalMax, S1_daily_LocalMin, S1_daily_RSI, min_div_period, max_div_period);
    /* Indicatori per S2 daily */
    Indicators::LocalMin *       S2_daily_LocalMin       = new Indicators::LocalMin      (dax_daily_instr->bars); 
    Indicators::LocalMax *       S2_daily_LocalMax       = new Indicators::LocalMax      (dax_daily_instr->bars);
    Indicators::RSI      *       S2_daily_RSI            = new Indicators::RSI           (dax_daily_instr->bars);
    Indicators::BollingerBands * S2_daily_BollingerBands = new Indicators::BollingerBands(dax_daily_instr->bars, 2.5, 2.5, 20); // NOTA: S2 utilizza le Bollinger a 2.5
    Indicators::Divergence *     S2_daily_Divergence     = new Indicators::Divergence    (dax_daily_instr->bars, S2_daily_LocalMax, S2_daily_LocalMin, S2_daily_RSI, min_div_period, max_div_period);
    Indicators::LongDivergence * S2_daily_LongDivergence = new Indicators::LongDivergence(dax_daily_instr->bars, S2_daily_LocalMax, S2_daily_LocalMin, S2_daily_RSI, min_div_period, max_div_period);
/* Utilizzando gli strumenti, le condizioni, e gli indicatori definiti qua sopra,
   inizializziamo le varie strategie (S1 e S2 daily e hourly) */
    Strategy * S1_daily = new Strategy(dax_daily_instr, client->m_tradeData, doubleToDecimal(1.0), "../backtesting/S1_Dax_daily.txt", "../backtesting/S1_Dax_daily.txt",
                                    2.5, 2.5, 18, /* Settare qua stop loss, take profit, e massime barre prima della chiusura in negativo*/
                                    "S1_daily", S1_daily_LocalMin, S1_daily_LocalMax, S1_daily_RSI, S1_daily_BollingerBands, S1_daily_Divergence, S1_daily_LongDivergence, S1_entry_conditions, S1a2_exit_conditions,
                                    SHORT, max_neg_period, true /* parametri per divergenza negata */
                                    );
    Strategy * S1_hourly_shortDiv = new Strategy(dax_hourly_instr, client->m_tradeData, doubleToDecimal(1.0), "../backtesting/S1_Dax_hourly_shortDiv.txt", "../backtesting/S1_Dax_hourly_shortDiv.txt",
                                    2.5, 2.5, 18, /* Settare qua stop loss, take profit, e massime barre prima della chiusura in negativo*/
                                    "S1_hourly_shortDiv", S1_hourly_LocalMin, S1_hourly_LocalMax, S1_hourly_RSI, S1_hourly_BollingerBands, S1_hourly_Divergence0, S1_hourly_LongDivergence0, S1_entry_conditions, S1a2_exit_conditions,
                                    SHORT, max_neg_period, true /* parametri per divergenza negata */
                                    );
    Strategy * S1_hourly_longDiv = new Strategy(dax_hourly_instr, client->m_tradeData, doubleToDecimal(1.0), "../backtesting/S1_Dax_hourly_longDiv.txt", "../backtesting/S1_Dax_hourly_longDiv.txt",
                                    2.5, 2.5, 18, /* Settare qua stop loss, take profit, e massime barre prima della chiusura in negativo*/
                                    "S1_hourly_longDiv", S1_hourly_LocalMin, S1_hourly_LocalMax, S1_hourly_RSI, S1_hourly_BollingerBands, S1_hourly_Divergence1, S1_hourly_LongDivergence1, S1_entry_conditions, S1a2_exit_conditions,
                                    LONG, max_neg_period, true /* parametri per divergenza negata */
                                    );

    Strategy * S2_hourly = new Strategy(dax_hourly_instr, client->m_tradeData, doubleToDecimal(1.0), "../backtesting/S2_Dax_hourly.txt", "../backtesting/S2_Dax_hourly.txt",
                                    3.5, 3.5, 63, /* Settare qua stop loss, take profit, e massime barre prima della chiusura in negativo*/
                                    "S2_hourly", S2_hourly_LocalMin, S2_hourly_LocalMax, S2_hourly_RSI, S2_hourly_BollingerBands, S2_hourly_Divergence, S2_hourly_LongDivergence, S2_entry_conditions, S1a2_exit_conditions
                                    );
    Strategy * S2_daily = new Strategy(dax_daily_instr, client->m_tradeData, doubleToDecimal(1.0), "../backtesting/S2_Dax_daily.txt", "../backtesting/S2_Dax_daily.txt",
                                    3.5, 3.5, 63, /* Settare qua stop loss, take profit, e massime barre prima della chiusura in negativo*/
                                    "S2_daily", S2_daily_LocalMin, S2_daily_LocalMax, S2_daily_RSI, S2_daily_BollingerBands, S2_daily_Divergence, S2_daily_LongDivergence, S2_entry_conditions, S1a2_exit_conditions
                                    );
/* Aggiungiamo le strategie */
    client->add_Strategy(dax_hourly_id, S1_hourly_shortDiv);
    client->add_Strategy(dax_hourly_id, S1_hourly_longDiv);
    //client->add_Strategy(dax_hourly_id, S2_hourly);
    //client->add_Strategy(dax_daily_id, S1_daily);
    //client->add_Strategy(dax_daily_id, S2_daily);
/* Esegui backtesting utilizzando i dati degli ultimi 3 anni */
    client->setTradingState(BACKTESTING);
    /* aggiorna dati contratti */
    client->update_contracts(); 
    /* Determina il numero di barre da includere nei backtest */
    client->update_instr_bars(dax_hourly_id, 32000, true);
    // client->update_instr_bars(dax_daily_id, 4096, true);
    /* stampa risultati backtests */
    client->print_backtests(); 
/* Elimina strategie */
#ifdef INDDBG
    S1_hourly_shortDiv->print_indicators();
#endif
    delete (S1_hourly_shortDiv); /* Can only delete one strategy if same Inidcator pointers held by two different strategies! */
    delete (S1_daily);
    delete (S2_hourly);
    delete (S2_daily);
}

void run_livetrades(MClient * client) {
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
    Strategy * S1_short = new Strategy(dax_short_instr, client->m_tradeData, doubleToDecimal(1.0), "../backtesting/S1_Dax_short.txt", "../backtesting/S1_Dax_short.txt",
                                    2.5, 2.5, 18, /* Settare qua stop loss, take profit, e massime barre prima della chiusura in negativo*/
                                    "S1_short", S1_short_LocalMin, S1_short_LocalMax, S1_short_RSI, S1_short_BollingerBands, S1_short_Divergence, S1_short_LongDivergence, S1_entry_conditions, S1a2_exit_conditions
                                    );
    Strategy * S2_short = new Strategy(dax_short_instr, client->m_tradeData, doubleToDecimal(1.0), "../backtesting/S2_Dax_short.txt", "../backtesting/S2_Dax_short.txt",
                                    3.5, 3.5, 63, /* Settare qua stop loss, take profit, e massime barre prima della chiusura in negativo*/
                                    "S2_short", S2_short_LocalMin, S2_short_LocalMax, S2_short_RSI, S2_short_BollingerBands, S2_short_Divergence, S2_short_LongDivergence, S2_entry_conditions, S1a2_exit_conditions
                                    );
    client->add_Strategy(dax_short_id, S1_short);
    client->add_Strategy(dax_short_id, S2_short);
    client->update_contracts(); 
    // To test live trading on bar retrieval data:
    client->setTradingState(LIVE);
    client->update_bars(4096, true); 
    client->setTradingState(LIVE); // SET LIVE TRADING STATE AFTER ADDING STRATEGY AND AFTER INITIALIZING BARS!
    client->reqRealTimeBars(dax_short_instr->m_reqIds.realTimeBars,
                            dax_short_instr->dataContract.contract,
                            -1, // "currently ignored", realtimebars at 5 second-intervals
                            "TRADES",
                            1);
    // Enter loop, trying to update bars every 5 seconds
    int dur_elapsed = 0;
    const int update_frequency = 5;
    const int max_dur = 10800; // 3 hours
    while (dur_elapsed < max_dur) {
        /** 20S = invalid duration */
        client->update_bars(12, false);
        std::this_thread::sleep_for(std::chrono::seconds(update_frequency));
        dur_elapsed += update_frequency;
    }
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
