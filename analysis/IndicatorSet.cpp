
#include "Indicators.h"
#include "Bars.h"

#define MIN_DIV_PERIOD 2
#define MAX_DIV_PERIOD 14
/******* DECLARATION ********/

class IndicatorSet
{
    public:

    /******* INDICATOR INSTANCES, CONSTRUCTOR, DELETE ********/

    IndicatorSet(Bars*);
    ~IndicatorSet();
    void printIndicators();

    Indicators::RSI*            RSI;
    Indicators::LocalMin*       LocalMin;
    Indicators::LocalMax*       LocalMax;
    Indicators::Divergence*     Divergence;
    Indicators::LongDivergence* LongDivergence;
    Indicators::BollingerBands* BollingerBands;
    Indicators::JCandleSticks*  JCandleSticks;
};

/******* IMPLEMENTATION ********/

IndicatorSet::IndicatorSet(::Bars* Bars)
{
    this->RSI            = new Indicators::RSI(Bars, 0);            this->RSI->computeIndicator();
    this->LocalMax       = new Indicators::LocalMax(Bars, 0);       this->LocalMax->computeIndicator();
    this->LocalMin       = new Indicators::LocalMin(Bars, 0);       this->LocalMin->computeIndicator();
    this->BollingerBands = new Indicators::BollingerBands(Bars, 0); this->BollingerBands->computeIndicator();
    this->JCandleSticks  = new Indicators::JCandleSticks(Bars, 0);  this->JCandleSticks->computeIndicator();
    this->Divergence     = new Indicators::Divergence(Bars,
        this->LocalMax, this->LocalMin, this->RSI, MIN_DIV_PERIOD, MAX_DIV_PERIOD);                  this->Divergence->computeIndicator();
    this->LongDivergence = new Indicators::LongDivergence(Bars, 
        this->LocalMax, this->LocalMin, this->RSI, MIN_DIV_PERIOD, MAX_DIV_PERIOD);                  this->LongDivergence->computeIndicator();
}

IndicatorSet::~IndicatorSet()
{
    delete(this->LongDivergence);
    delete(this->Divergence);
    delete(this->BollingerBands);
    delete(this->LocalMax);
    delete(this->LocalMin);
    delete(this->RSI);
    delete(this->JCandleSticks);
}

void IndicatorSet::printIndicators()
{
    this->LongDivergence->printIndicator();
    this->Divergence->printIndicator();
    this->BollingerBands->printIndicator();
    this->LocalMax->printIndicator();
    this->LocalMin->printIndicator();
    this->RSI->printIndicator();
    this->JCandleSticks->printIndicator();
}


//*** TESTING ***//

#ifdef INDTEST
int main()
{
    IndicatorSet* Iset = new IndicatorSet(new Bars("../input/DAXINDEX_Start07042020_End_0604023.txt"));
    // fprintf(stdout,"%s\n", (Iset->LongDivergence->getIndicatorBar(100)->logString()).c_str());
    Iset->printIndicators();
    delete(Iset);
    return 0;
}
#endif //INDTEST
