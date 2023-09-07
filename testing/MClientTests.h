
#pragma once
#include "../myclient/MClient.h"
#include <chrono>

/* Include relevant header file based on what test has been defined in the main CommonMacros header */
#if defined(BACKTEST)
    #include "BacktestParams.h"
#elif defined(LIVETRADE) || defined(TRADINGLOOP)
    #include "testTradeParams.h"
#else
    #error  "Define a testing mode: BACKTEST, LIVETRADE, or TRADINGLOOP"
#endif

/* Define instruments, indicators, and strategies as global uninitialized variables */
static int                             inst_id[3];     
static Instrument *                    instr[3];           
static Indicators::LocalMin *          localMin[3];       
static Indicators::LocalMax *          localMax[3];         
static Indicators::RSI      *          Rsi[3];            
static Indicators::BollingerBands *    bollBands[3];      
static Indicators::Divergence *        divergence[3];     
static Indicators::LongDivergence *    longDivergence[3];  
static Strategy *                      strat[3];        

/* Macros */
#define ADD_INDICATORS(instr, localMin, localMax, Rsi, bollBands, divergence, longDivergence ) \
/* Indicators::LocalMin *       */  localMin       = new Indicators::LocalMin      ( instr -> bars); \
/* Indicators::LocalMax *       */  localMax       = new Indicators::LocalMax      ( instr -> bars); \
/* Indicators::RSI      *       */  Rsi            = new Indicators::RSI           ( instr -> bars); \
/* Indicators::BollingerBands * */  bollBands      = new Indicators::BollingerBands( instr -> bars, stDevUp, stDevDown, boll_time_period); \
/* Indicators::Divergence *     */  divergence     = new Indicators::Divergence    ( instr -> bars, localMax, localMin, Rsi, min_div_period, max_div_period); \
/* Indicators::LongDivergence * */  longDivergence = new Indicators::LongDivergence( instr -> bars, localMax, localMin, Rsi, min_div_period, max_div_period);

#define INIT_STRATEGY(strat, strategy_code, instr, localMin, localMax, Rsi, bollBands, divergence, longDivergence ) \
    /* Strategy * */ strat = new Strategy(instr, client->m_tradeData, stop_loss, take_profit, exp_bars, \
        strategy_code, entry_conditions, exit_conditions, localMin, localMax, Rsi, bollBands, divergence, \
        longDivergence, 1.0, divType, max_neg_period, RSI_cond, Boll_cond /* parametri per divergenza negata */);

static void init_strategies(MClient * client);
static void print_data(MClient * client);
static void del_strategies();

void test_contract_details(MClient * client);
void run_backtests(MClient * client);
void run_livetrades(MClient * client); 
void durationStr_test(MClient * client);
void trading_loop(MClient * client, int loop_dur);

