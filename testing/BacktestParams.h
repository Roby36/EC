
#pragma once
#include "../myclient/MClient.h"

/** NOTA: 
 * I backtest possono essere eseguiti definendo i seguenti parametri in questo file:
 *      1) I parametri completi per 1 sola strategia
 *      2) Più diversi mercati su cui applicare la strategia
*/

/* Definisci diversi mercati (ad esempio qua ne aggiungiamo 3)*/
// Questo parametro possiamo chiamarlo come vogliamo
static const std::string strategy_code_1 = "DAX_S2_hourly_longDiv";  /* nome strategia  */
// Questo parametro deve seguire un format particolare richiesto dalla piattaforma (screenshot_3)
static const std::string barSize_1 = "1 hour";                       /* grandezza barre */
// QUesti parametri sono funzioni definite da noi, nella caretlla conyracts/MContractDetails.cpp
static const ContractDetails dataContract_1  = MContractDetails::DAXInd();  /* contratto dati (DAX indice) */
static const ContractDetails orderContract_1 = MContractDetails::DAXFut(); /* contratto ordini (DAX future) */

#ifndef SS_TEST /* Per testare una singola strategia evitando di aggiungerne altre */
static const std::string strategy_code_2 = "ESTX50_S2_hourly_longDiv";  /* nome strategia  */
static const std::string barSize_2 = "1 hour";                       /* grandezza barre */
static const ContractDetails dataContract_2  = MContractDetails::ESTX50Index();  /* contratto dati */
static const ContractDetails orderContract_2 = MContractDetails::ESTX50Future(); /* contratto ordini */

static const std::string strategy_code_3 = "NIKKEI_S2_daily_longDiv";  /* nome strategia  */
static const std::string barSize_3 = "1 day";                       /* grandezza barre */
static const ContractDetails dataContract_3  = MContractDetails::NIKInd();  /* contratto dati */
static const ContractDetails orderContract_3 = MContractDetails::NIKFut(); /* contratto ordini */
#endif 

/* Ora definiamo i parametri della strategia da applicare su tutti mercati definiti sopra */
static EntryConditions entry_conditions[] = {DOUBLE_DIVERGENCE, 
                                            ENTRY_CONDITIONS_END}; 
                                            //! Ricorda di non eliminare "ENTRY_CONDITIONS_END" alla fine della lista
static ExitConditions  exit_conditions [] = {OPPOSITE_DIVERGENCE,            
                                            BOLLINGER_CROSSING, 
                                            NEGATIVE_TRADE_EXPIRATION, 
                                            STOP_LOSS_TAKE_PROFIT, 
                                            EXIT_CONDITIONS_END};
                                            //! Ricorda di non eliminare "EXIT_CONDITIONS_END" alla fine della lista
// Parametri indicatori 
static const double stDevUp   = 2.0; /* parametri bande Bollinger */
static const double stDevDown = 2.0;
static const int boll_time_period = 14; 
static const int min_div_period =  1;  /* minimo peiodo tra i due massimi/minimi di una divergenza */
static const int max_div_period = 18;  /* massimo peiodo tra i due massimi/minimi di una divergenza */

// Parametri backtest
static const int num_bars = 20000;     /* numero di barre da richiedere per il backtesting */
static const std::string outputDir = "../graphs/data/"; /* dove stampare risultati backtest per grafici (non serve cambiarla)*/

// Parametri strategia
static const int max_neg_period = 18;  /* SOLO per S1: massimo peiodo (in barre) per la negazione */
static const double take_profit = 4.0;
static const double stop_loss   = 3.0;
static const int exp_bars = 18;
static const DivergenceType divType = LONG_DIV; /* tipo di divergenza (breve o lunga) */
static const RSI_condition RSI_cond = LSTAT_LBAR; /* condizione RSI per definire divergenza negata (e.g. barra più a sinistra in assoluto) */
static const BOLLINGER_CONDITION Boll_cond = MIDDLE_BAND; /* condizione attraversamento Bollinger */

