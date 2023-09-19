
#pragma once
/* Strategy.h */


enum TradingState {
	BACKTESTING,
	LIVE,
	RETRIEVAL
};

enum EntryConditions {
	DENIED_DIVERGENCE,
	DOUBLE_DIVERGENCE,
	ENTRY_CONDITIONS_END
};

enum ExitConditions {
	OPPOSITE_DIVERGENCE,
	BOLLINGER_CROSSING,
	NEGATIVE_TRADE_EXPIRATION,
	STOP_LOSS_TAKE_PROFIT,
	EXIT_CONDITIONS_END
};

enum DivergenceType {
	SHORT_DIV,
	LONG_DIV
};

enum StatType {
	MAX = -1,
	MIN = 1
};

enum RSI_condition {
	LSTAT_LBAR, /* massimo / minimo SINISTRO della prima divergenza della serie di divergenze = PRIMO massimo della sequenza di divergenze */
	LSTAT_RBAR, /* massimo / minimo DESTRO della prima divergenza della serie di divergenze   = SECONDO massimo della sequenza di divergenze */
	NONE        /* Nessuna restrizione riguardo i valori RSI */
};

enum BOLLINGER_CONDITION {
	MIDDLE_BAND,
	OUTER_BANDS
};

/* MTrade.h */

enum TradeDirection {
    SHORT = -1,
    LONG  = 1
};

/* MClient.h */

enum State {
    ST_CONNECT,
    REQCONTRACTDETAILS,
    REQCONTRACTDETAILS_ACK,
    REQHISTORICALDATA,
    REQHISTORICALDATA_ACK,
    REQIDS,
    REQIDS_ACK,
    PLACEORDER,
    PLACEORDER_ACK,
    CANCELORDER,
    CANCELORDER_ACK,
    REQALLOPENORDERS,
    REQALLOPENORDERS_ACK,
    REQCOMPLETEDORDERS,
    REQCOMPLETEDORDERS_ACK,
    REQREALTIMEBARS,
    REQREALTIMEBARS_ACK,
    REQPOSITIONS,
    REQPOSITIONS_ACK,
    REQEXECUTIONS,
    REQEXECUTIONS_ACK

};

