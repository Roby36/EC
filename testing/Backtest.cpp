
#include "MClientTests.h"

/** PARAMETRI: */

static const std::string strategy_code = "DAX_S2_daily_longDiv";  /* nome strategia  */
static const std::string barSize = "1 day";                       /* grandezza barre */
static const ContractDetails dataContract  = MContractDetails::DAXInd();
static const ContractDetails orderContract = MContractDetails::DAXFut();
/* Definisci le condizioni di entrata e uscita (mettendo sempre "ENTRY_CONDITIONS_END" alla fine della lista)*/
static EntryConditions entry_conditions[] = {DOUBLE_DIVERGENCE, ENTRY_CONDITIONS_END}; 
static ExitConditions  exit_conditions [] = {OPPOSITE_DIVERGENCE,                     
                                            BOLLINGER_CROSSING, 
                                            NEGATIVE_TRADE_EXPIRATION, 
                                            STOP_LOSS_TAKE_PROFIT, 
                                            EXIT_CONDITIONS_END};
static const DivergenceType divType = LONG; /* tipo di divergenza (breve o lunga) */
static const int num_bars = 20000;     /* numero di barre da richiedere per il backtesting */
static const int min_div_period =  1;  /* minimo peiodo tra i due massimi/minimi di una divergenza */
static const int max_div_period = 7;  /* massimo peiodo tra i due massimi/minimi di una divergenza */
static const int max_neg_period = 7;  /* massimo peiodo (in barre) per la negazione */
static const double take_profit = 5.0;
static const double stop_loss   = 5.0;
static const int exp_bars = 7;
static const RSI_condition RSI_cond = LSTAT_LBAR; /* condizione RSI per definire divergenza negata (e.g. barra più a sinistra in assoluto) */
static const BOLLINGER_CONDITION Boll_cond = MIDDLE_BAND; /* condizione attraversamento Bollinger */
static const double stDevUp   = 2.0; /* parametri bande Bollinger */
static const double stDevDown = 2.0;
static const int boll_time_period = 14; 

const std::string outputDir = "../graphs/data/"; /* dove stampare risultati backtest */

void run_backtests(MClient * client) {
/* Aggiungi strumento */
    int inst_id = client->add_Instrument(barSize, dataContract, orderContract,
                                    Instrument::ReqIds(6101, 6201, 6301, 6401, 6501), "../instruments_log/backtest_instr.txt"); 
    Instrument * instr = client->get_Instrument(inst_id);
/* Inizializza gli indicatori */    
    Indicators::LocalMin *       localMin       = new Indicators::LocalMin      (instr->bars); 
    Indicators::LocalMax *       localMax       = new Indicators::LocalMax      (instr->bars);
    Indicators::RSI      *       Rsi            = new Indicators::RSI           (instr->bars);
    Indicators::BollingerBands * bollBands      = new Indicators::BollingerBands(instr->bars, stDevUp, stDevDown, boll_time_period); 
    Indicators::Divergence *     divergence     = new Indicators::Divergence    (instr->bars, localMax, localMin, Rsi, min_div_period, max_div_period);
    Indicators::LongDivergence * longDivergence = new Indicators::LongDivergence(instr->bars, localMax, localMin, Rsi, min_div_period, max_div_period);
/* Utilizzando gli strumenti, le condizioni, e gli indicatori definiti qua sopra, inizializziamo strategia */
    Strategy * strat = new Strategy(instr, client->m_tradeData, stop_loss, take_profit, exp_bars,
                                    strategy_code, entry_conditions, exit_conditions,
                                    localMin, localMax, Rsi, bollBands, divergence, longDivergence, 
                                    divType, max_neg_period, RSI_cond, Boll_cond /* parametri per divergenza negata */
                                    );
/* Aggiungi strategia */
    client->add_Strategy(inst_id, strat);
/* Esegui backtesting*/
    client->set_trading_state(BACKTESTING);
    /* aggiorna dati contratti */
    client->update_contracts(); 
    /* Determina il numero di barre da includere nei backtest */
    client->update_instr_bars(inst_id, num_bars, true);
    /* stampa risultati backtests */
    client->print_indicators(outputDir);
    client->print_bars(outputDir);
    client->print_PL_data(outputDir);
    client->print_backtest_results();
/* Elimina strategia */
    delete (strat);
}
