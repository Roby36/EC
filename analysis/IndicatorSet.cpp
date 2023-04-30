
#include "Indicators.cpp"
#include "Bars.cpp"

/******* DECLARATION ********/

class IndicatorSet
{
    
    public:

    /******* INDICATOR INSTANCES, CONSTRUCTOR, DELETE ********/

    IndicatorSet(Bars*);
    void Delete();
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
    this->RSI            = new Indicators::RSI(Bars);            this->RSI->computeIndicator();
    this->LocalMax       = new Indicators::LocalMax(Bars);       this->LocalMax->computeIndicator();
    this->LocalMin       = new Indicators::LocalMin(Bars);       this->LocalMin->computeIndicator();
    this->BollingerBands = new Indicators::BollingerBands(Bars); this->BollingerBands->computeIndicator();
    this->JCandleSticks  = new Indicators::JCandleSticks(Bars);  this->JCandleSticks->computeIndicator();
    this->Divergence     = new Indicators::Divergence(Bars, 
        this->LocalMax, this->LocalMin, this->RSI);              this->Divergence->computeIndicator();
    this->LongDivergence = new Indicators::LongDivergence(Bars,
        this->LocalMax, this->LocalMin, this->RSI);              this->LongDivergence->computeIndicator();
}

void IndicatorSet::Delete()
{
    this->LongDivergence->Delete(); delete(this->LongDivergence);
    this->Divergence->Delete();     delete(this->Divergence);
    this->BollingerBands->Delete(); delete(this->BollingerBands);
    this->LocalMax->Delete();       delete(this->LocalMax);
    this->LocalMin->Delete();       delete(this->LocalMin);
    this->RSI->Delete();            delete(this->RSI);
    this->JCandleSticks->Delete();  delete(this->JCandleSticks);

    delete(this);
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
    IndicatorSet* Iset = new IndicatorSet(new Bars());
    // fprintf(stdout,"%s\n", (Iset->LongDivergence->getIndicatorBar(100)->logString()).c_str());
    Iset->printIndicators();
    Iset->Delete();
    return 0;
}
#endif //INDTEST
