
#pragma once
#include "../myclient/MClient.h"

static const std::string strategy_code_1 = "DAX_S1_5sec_fut_longDiv";  /* nome strategia  */
static const std::string barSize_1 = "5 secs";                       /* grandezza barre */
static const ContractDetails dataContract_1  = MContractDetails::DAXFut();  /* contratto dati */
static const ContractDetails orderContract_1 = MContractDetails::DAXFut(); /* contratto ordini */

#ifndef SS_TEST
static const std::string strategy_code_2 = "ESTX_S1_5sec_fut_longDiv";  /* nome strategia  */
static const std::string barSize_2 = "5 secs";                       /* grandezza barre */
static const ContractDetails dataContract_2  = MContractDetails::ESTX50Future();  /* contratto dati */
static const ContractDetails orderContract_2 = MContractDetails::ESTX50Future(); /* contratto ordini */

static const std::string strategy_code_3 = "NIKKEI_S1_5sec_fut_longDiv";  /* nome strategia  */
static const std::string barSize_3 = "5 secs";                               /* grandezza barre */
static const ContractDetails dataContract_3  = MContractDetails::NIKFut();  /* contratto dati */
static const ContractDetails orderContract_3 = MContractDetails::NIKFut(); /* contratto ordini */
#endif 

static EntryConditions entry_conditions[] = {DENIED_DIVERGENCE, ENTRY_CONDITIONS_END}; 
static ExitConditions  exit_conditions [] = {OPPOSITE_DIVERGENCE,            
                                            BOLLINGER_CROSSING, 
                                            NEGATIVE_TRADE_EXPIRATION, 
                                            STOP_LOSS_TAKE_PROFIT, 
                                            EXIT_CONDITIONS_END};
static const DivergenceType divType = LONG_DIV; 
static const int num_bars = 1024;               // retrieval bars to gather initial data     
static const int min_div_period =  1;  
static const int max_div_period = 18;  
static const int max_neg_period = 18;  
static const double take_profit = 0.01;         // very low to test with high frequency bars
static const double stop_loss   = 0.01;
static const int exp_bars = 18;
static const RSI_condition RSI_cond = LSTAT_LBAR; 
static const BOLLINGER_CONDITION Boll_cond = MIDDLE_BAND; 
static const double stDevUp   = 2.0; 
static const double stDevDown = 2.0;
static const int boll_time_period = 14; 

static const std::string outputDir = "../graphs/data/";