
#include "MClient.h"
#include <chrono>

/* Rimuovi il commento "//" per attivare i backtest (che verranno definiti nella funzione seguente) */
// #define BACKTEST
#define LIVETRADE

void run_backtests(MClient * client) {
/* Aggiungi strumento DAX hourly*/
    int dax_hourly_id = client->add_Instrument( "1 hour", MContractDetails::DAXInd(), MContractDetails::DAXFut(),    
                                    Instrument::ReqIds(3101, 3201, 3301, 3401, 3501), "Dax_hourly.txt");
/* Aggiungi strumento DAX daily*/
    int dax_daily_id = client->add_Instrument( "1 day", MContractDetails::DAXInd(), MContractDetails::DAXFut(),    
                                    Instrument::ReqIds(4101, 4201, 4301, 4401, 4501), "Dax_daily.txt");
/* Definisci le condizioni di entrata e uscita (mettendo sempre "ENTRY_CONDITIONS_END" alla fine della lista)*/
    EntryConditions S1_entry_conditions  [] = {DENIED_DIVERGENCE, ENTRY_CONDITIONS_END}; // entrata per S1
    EntryConditions S2_entry_conditions  [] = {DOUBLE_DIVERGENCE, ENTRY_CONDITIONS_END}; // entrata per S2
    ExitConditions  S1a2_exit_conditions []  = {OPPOSITE_DIVERGENCE,                     // uscita per S1 e S2
                                                BOLLINGER_CROSSING, 
                                                NEGATIVE_TRADE_EXPIRATION, 
                                                STOP_LOSS_TAKE_PROFIT, 
                                                EXIT_CONDITIONS_END};
/* Inizializza gli indicatori per ogni strategia, e per ogni grandezza di barre (4 blocchi in totale) */
/* Qua ad esempio decidiamo l'ampiezza delle Bollinger ed altre variabili degli indicatori,
/* che poi verranno prese in considerazione per dare i vari segnali di entrata e uscita */
    Instrument * dax_hourly_instr = client->get_Instrument(dax_hourly_id);
    Instrument * dax_daily_instr  = client->get_Instrument(dax_daily_id);
    /* Indicatori per S1 hourly */
    Indicators::LocalMin *       S1_hourly_LocalMin       = new Indicators::LocalMin      (dax_hourly_instr->bars); 
    Indicators::LocalMax *       S1_hourly_LocalMax       = new Indicators::LocalMax      (dax_hourly_instr->bars);
    Indicators::RSI      *       S1_hourly_RSI            = new Indicators::RSI           (dax_hourly_instr->bars);
    Indicators::BollingerBands * S1_hourly_BollingerBands = new Indicators::BollingerBands(dax_hourly_instr->bars, 2.0, 2.0, 20); // NOTA: S1 utilizza le Bollinger a 2.0
    Indicators::LongDivergence * S1_hourly_LongDivergence = new Indicators::LongDivergence(dax_hourly_instr->bars, S1_hourly_LocalMax, S1_hourly_LocalMin, S1_hourly_RSI);
    /* Indicatori per S2 hourly */
    Indicators::LocalMin *       S2_hourly_LocalMin       = new Indicators::LocalMin      (dax_hourly_instr->bars); 
    Indicators::LocalMax *       S2_hourly_LocalMax       = new Indicators::LocalMax      (dax_hourly_instr->bars);
    Indicators::RSI      *       S2_hourly_RSI            = new Indicators::RSI           (dax_hourly_instr->bars);
    Indicators::BollingerBands * S2_hourly_BollingerBands = new Indicators::BollingerBands(dax_hourly_instr->bars, 2.5, 2.5, 20); // NOTA: S2 utilizza le Bollinger a 2.5
    Indicators::LongDivergence * S2_hourly_LongDivergence = new Indicators::LongDivergence(dax_hourly_instr->bars, S2_hourly_LocalMax, S2_hourly_LocalMin, S2_hourly_RSI);
    /* Indicatori per S1 daily*/
    Indicators::LocalMin *       S1_daily_LocalMin       = new Indicators::LocalMin      (dax_daily_instr->bars); 
    Indicators::LocalMax *       S1_daily_LocalMax       = new Indicators::LocalMax      (dax_daily_instr->bars);
    Indicators::RSI      *       S1_daily_RSI            = new Indicators::RSI           (dax_daily_instr->bars);
    Indicators::BollingerBands * S1_daily_BollingerBands = new Indicators::BollingerBands(dax_daily_instr->bars, 2.0, 2.0, 20); // NOTA: S1 utilizza le Bollinger a 2.0
    Indicators::LongDivergence * S1_daily_LongDivergence = new Indicators::LongDivergence(dax_daily_instr->bars, S1_daily_LocalMax, S1_daily_LocalMin, S1_daily_RSI);
    /* Indicatori per S2 daily */
    Indicators::LocalMin *       S2_daily_LocalMin       = new Indicators::LocalMin      (dax_daily_instr->bars); 
    Indicators::LocalMax *       S2_daily_LocalMax       = new Indicators::LocalMax      (dax_daily_instr->bars);
    Indicators::RSI      *       S2_daily_RSI            = new Indicators::RSI           (dax_daily_instr->bars);
    Indicators::BollingerBands * S2_daily_BollingerBands = new Indicators::BollingerBands(dax_daily_instr->bars, 2.5, 2.5, 20); // NOTA: S2 utilizza le Bollinger a 2.5
    Indicators::LongDivergence * S2_daily_LongDivergence = new Indicators::LongDivergence(dax_daily_instr->bars, S2_daily_LocalMax, S2_daily_LocalMin, S2_daily_RSI);
/* Utilizzando gli strumenti, le condizioni, e gli indicatori definiti qua sopra,
/* inizializziamo le varie strategie (S1 e S2 daily e hourly) */
    Strategy * S1_daily = new Strategy(dax_daily_instr, client->m_tradeData, doubleToDecimal(1.0), "../backtesting/S1_Dax_daily.txt", "../backtesting/S1_Dax_daily.txt",
                                    2.5, 2.5, 18, /* Settare qua stop loss, take profit, e massime barre prima della chiusura in negativo*/
                                    "S1_daily", S1_daily_LocalMin, S1_daily_LocalMax, S1_daily_RSI, S1_daily_BollingerBands, S1_daily_LongDivergence, S1_entry_conditions, S1a2_exit_conditions
                                    );
    Strategy * S2_daily = new Strategy(dax_daily_instr, client->m_tradeData, doubleToDecimal(1.0), "../backtesting/S2_Dax_daily.txt", "../backtesting/S2_Dax_daily.txt",
                                    3.5, 3.5, 63, /* Settare qua stop loss, take profit, e massime barre prima della chiusura in negativo*/
                                    "S2_daily", S2_daily_LocalMin, S2_daily_LocalMax, S2_daily_RSI, S2_daily_BollingerBands, S2_daily_LongDivergence, S2_entry_conditions, S1a2_exit_conditions
                                    );
    Strategy * S1_hourly = new Strategy(dax_hourly_instr, client->m_tradeData, doubleToDecimal(1.0), "../backtesting/S1_Dax_hourly.txt", "../backtesting/S1_Dax_hourly.txt",
                                    2.5, 2.5, 18, /* Settare qua stop loss, take profit, e massime barre prima della chiusura in negativo*/
                                    "S1_hourly", S1_hourly_LocalMin, S1_hourly_LocalMax, S1_hourly_RSI, S1_hourly_BollingerBands, S1_hourly_LongDivergence, S1_entry_conditions, S1a2_exit_conditions
                                    );
    Strategy * S2_hourly = new Strategy(dax_hourly_instr, client->m_tradeData, doubleToDecimal(1.0), "../backtesting/S2_Dax_hourly.txt", "../backtesting/S2_Dax_hourly.txt",
                                    3.5, 3.5, 63, /* Settare qua stop loss, take profit, e massime barre prima della chiusura in negativo*/
                                    "S2_hourly", S2_hourly_LocalMin, S2_hourly_LocalMax, S2_hourly_RSI, S2_hourly_BollingerBands, S2_hourly_LongDivergence, S2_entry_conditions, S1a2_exit_conditions
                                    );
/* Aggiungiamo le strategie */
    client->add_Strategy(dax_hourly_id, S1_hourly);
    client->add_Strategy(dax_hourly_id, S2_hourly);
    client->add_Strategy(dax_daily_id, S1_daily);
    client->add_Strategy(dax_daily_id, S2_daily);
/* Esegui backtesting utilizzando i dati degli ultimi 3 anni */
    client->setTradingState(BACKTESTING);
    client->update_contracts();  // aggiorna dati contratti
    client->initialize_bars("3 Y", "TRADES"); // richiedi dati per gli ultimi 3 anni
    client->print_backtests(); // stampa risultati backtests
/* Elimina strategie */
    delete (S1_hourly);
    delete (S1_daily);
    delete (S2_hourly);
    delete (S2_daily);

}

void run_livetrades(MClient * client) {
    int dax_short_id = client->add_Instrument( "5 secs", MContractDetails::DAXInd(), MContractDetails::DAXFut(),    
                                               Instrument::ReqIds(101, 201, 301, 401, 501), "Dax_short.txt");
    Instrument * dax_short_instr = client->get_Instrument(dax_short_id);
    client->setTradingState(LIVE);
    client->reqRealTimeBars(dax_short_instr->m_reqIds.realTimeBars,
                            dax_short_instr->dataContract.contract,
                            -1, // "currently ignored", realtimebars at 5 second-intervals
                            "TRADES",
                            1);
    EntryConditions S1_entry_conditions  [] = {DENIED_DIVERGENCE, ENTRY_CONDITIONS_END}; 
    EntryConditions S2_entry_conditions  [] = {DOUBLE_DIVERGENCE, ENTRY_CONDITIONS_END};
    ExitConditions  S1a2_exit_conditions []  = {OPPOSITE_DIVERGENCE,                     
                                                BOLLINGER_CROSSING, 
                                                NEGATIVE_TRADE_EXPIRATION, 
                                                STOP_LOSS_TAKE_PROFIT, 
                                                EXIT_CONDITIONS_END};
    /* S1 indicators */
    Indicators::LocalMin *       S1_short_LocalMin       = new Indicators::LocalMin      (dax_short_instr->bars); 
    Indicators::LocalMax *       S1_short_LocalMax       = new Indicators::LocalMax      (dax_short_instr->bars);
    Indicators::RSI      *       S1_short_RSI            = new Indicators::RSI           (dax_short_instr->bars);
    Indicators::BollingerBands * S1_short_BollingerBands = new Indicators::BollingerBands(dax_short_instr->bars, 2.0, 2.0, 20); // NOTA: S1 utilizza le Bollinger a 2.0
    Indicators::LongDivergence * S1_short_LongDivergence = new Indicators::LongDivergence(dax_short_instr->bars, S1_short_LocalMax, S1_short_LocalMin, S1_short_RSI);
    /* S2 indicators */
    Indicators::LocalMin *       S2_short_LocalMin       = new Indicators::LocalMin      (dax_short_instr->bars); 
    Indicators::LocalMax *       S2_short_LocalMax       = new Indicators::LocalMax      (dax_short_instr->bars);
    Indicators::RSI      *       S2_short_RSI            = new Indicators::RSI           (dax_short_instr->bars);
    Indicators::BollingerBands * S2_short_BollingerBands = new Indicators::BollingerBands(dax_short_instr->bars, 2.5, 2.5, 20); // NOTA: S2 utilizza le Bollinger a 2.5
    Indicators::LongDivergence * S2_short_LongDivergence = new Indicators::LongDivergence(dax_short_instr->bars, S2_short_LocalMax, S2_short_LocalMin, S2_short_RSI);
    /* Strategies */
    Strategy * S1_short = new Strategy(dax_short_instr, client->m_tradeData, doubleToDecimal(1.0), "../backtesting/S1_Dax_short.txt", "../backtesting/S1_Dax_short.txt",
                                    2.5, 2.5, 18, /* Settare qua stop loss, take profit, e massime barre prima della chiusura in negativo*/
                                    "S1_short", S1_short_LocalMin, S1_short_LocalMax, S1_short_RSI, S1_short_BollingerBands, S1_short_LongDivergence, S1_entry_conditions, S1a2_exit_conditions
                                    );
    Strategy * S2_short = new Strategy(dax_short_instr, client->m_tradeData, doubleToDecimal(1.0), "../backtesting/S2_Dax_short.txt", "../backtesting/S2_Dax_short.txt",
                                    3.5, 3.5, 63, /* Settare qua stop loss, take profit, e massime barre prima della chiusura in negativo*/
                                    "S2_short", S2_short_LocalMin, S2_short_LocalMax, S2_short_RSI, S2_short_BollingerBands, S2_short_LongDivergence, S2_entry_conditions, S1a2_exit_conditions
                                    );
    client->add_Strategy(dax_short_id, S1_short);
    client->add_Strategy(dax_short_id, S2_short);
    client->update_contracts();  // aggiorna dati contratti
    client->initialize_bars("1 Y", "TRADES"); // richiedi dati per gli ultimi 3 anni
    // Enter loop, trying to update bars every 5 seconds
    int dur_elapsed = 0;
    const int update_frequency = 1;
    const int max_dur = 10800; // 3 hours
    while (dur_elapsed < max_dur) {
        client->update_bars("TRADES", 1, 12);
        std::this_thread::sleep_for(std::chrono::seconds(update_frequency));
        dur_elapsed += update_frequency;
    }
    delete(S1_short);
    delete(S2_short);

}

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

    // Give more time
    std::this_thread::sleep_for(std::chrono::seconds(5));

    /** BACKTESTING: */
#ifdef BACKTEST
    run_backtests(client);
#endif

    /** INSTRUMENTS: */
    // Set up an instrument at low time interval for testing
#ifndef LIVETRADE
    int inst1_id = client->add_Instrument( "5 secs", MContractDetails::CryptoContract(), MContractDetails::CryptoContract(),    
                                            Instrument::ReqIds(1101, 1201, 1301, 1401, 1501), "Btc.txt");
    int inst2_id = client->add_Instrument( "5 secs", MContractDetails::EurGbpFx(), MContractDetails::EurGbpFx(),    
                    Instrument::ReqIds(2101, 2201, 2301, 2401, 2501), "EurGbp.txt");
#endif

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