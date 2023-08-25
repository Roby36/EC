
/*** Main.cpp macros ***/
/** Rimuovi il commento "//" per attivare i backtest (che verranno definiti nella funzione seguente) **/
#define BACKTEST
// #define LIVETRADE
// #define DURSTRTEST
// #define TEST_CONTRACT_DETAILS

/*** Bars.h macros ***/
#define OUTDIRCHAR 64
#define MAXBARS 65536   //2^16

/*** BackTester.h macros ***/
// YOU MUST ALLOCATE THE ARRAY STATICALLY
// ELSE COMPILER WON'T KNOW SIZE OF OBJECT
// HENCE HOW MUCH MEMORY TO ALLOCATE ON THE HEAP
// --> BUS ERROR!
#define MAXBTTRADES 1024

/*** Instrument.h macros  ***/
#define MAX_BARSIZE_STRING_LENGTH 16
#define MAX_TOTAL_SECONDS 86400
#define MAX_TOTAL_DAYS    365
#define MAX_TH_STRING_LENGTH 32

/*** MClient.h macros ***/
#define MAXINSTR 16
#define MAXSTRAT 16

/*** MTrade.h macros */
// Trades need cleaning up when array full to make up space!
#define MAXTRADES 1024 // MUST BE 1024 FOR CURRENTLY STORED SER FILE!!!

/*** Strategy.h macros ***/
#define MAXOPENTRADES 8
#define MAXCLOSETRADES 8
#define MAXENTRYCONDS 8
#define MAXEXITCONDS  8