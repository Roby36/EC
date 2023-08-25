
#pragma once
#include "../myclient/MClient.h"
#include <chrono>

void test_contract_details(MClient * client);
void run_backtests(MClient * client);
void run_livetrades(MClient * client, int loop_dur = 0); 
void durationStr_test(MClient * client);
