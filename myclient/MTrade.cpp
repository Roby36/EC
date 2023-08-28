
#include "MTrade.h"

MTrade::~MTrade() {
    DEL_IF_N_NULL(openingOrder);
    DEL_IF_N_NULL(closingOrder);
    DEL_IF_N_NULL(openingExecution);
    DEL_IF_N_NULL(closingExecution);
}

