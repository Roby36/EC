
// Problems making one single object file from multiple translation units
#pragma once
#include "Indicator.cpp"
#include "IndicatorBar.cpp"
#include <math.h>

/******* INDICATOR CLASSES ********/

namespace Indicators
{
    class RSI : public Indicator<IndicatorBars::RSI> 
    {
        public:

        const int timePeriod;

        RSI(Bars* dp, const std::string name = "RSI",  const int TimePeriod = 14) 
            : Indicator(dp, name), timePeriod(TimePeriod) 
        {
        }

        void computeIndicator();
    };

    class LocalMin : public Indicator<IndicatorBars::LocalStat>
    {
        protected:
        int m = 1;

        public:
        LocalMin(Bars* dp, const std::string name = "LocalMin") 
            : Indicator(dp, name)
        {
        }

        void computeIndicator();
    };

    class LocalMax : public LocalMin
    { 
        public:
        LocalMax(Bars* dp, const std::string name = "LocalMax") 
            : LocalMin(dp, name)
        { this->m = -1; }
    };

    class Divergence : public Indicator<IndicatorBars::Divergence>
    {
        protected:
        // Reference to required Indicators
        class LocalMax* LocalMax;
        class LocalMin* LocalMin;
        class RSI* RSI;

        public:
        /*** (Elementary) divergence parameters ***/
        const int minDivPeriod;
        const int maxDivPeriod;
        const int minRSItimePeriods = 10;

        Divergence(Bars* dp,
                   class LocalMax* LocalMax, 
                   class LocalMin* LocalMin, 
                   class RSI* RSI,
                   int minDivPeriod,
                   int maxDivPeriod,
                   const std::string name = "Divergence");

        void computeIndicator();
    };

    class LongDivergence : public Divergence
    {
        public:

        LongDivergence(Bars* dp, 
                      class LocalMax* LocalMax, 
                      class LocalMin* LocalMin, 
                      class RSI* RSI,
                      int minDivPeriod,
                      int maxDivPeriod,
                      const std::string name = "LongDivergence")
            : Divergence(dp, LocalMax, LocalMin, RSI, minDivPeriod, maxDivPeriod, name)
        {
        }

        void computeIndicator() override;
    };

    class BollingerBands : public Indicator<IndicatorBars::BollingerBands>
    {
        /*** Bollinger Bands parameters ***/
        double stDevUp, stDevDown;
        int timePeriod;

        public:

        BollingerBands(Bars* dp, 
                       double stDevUp = 2.0, 
                       double stDevDown = 2.0, 
                       int timePeriod = 20,
                       const std::string name = "BB"); 
            
        void computeIndicator();
    };

    class JCandleSticks : public Indicator<IndicatorBars::JCandleSticks>
    {
        /*** JCandleSticks parameters ***/
        double hammerSize;
        double dojiSize;
        int dojiExtremes;

        public:

        JCandleSticks(Bars* dp,
                      double hammerSize = 3.0, 
                      double dojiSize = 8.0, 
                      int dojiExtremes = 3,
                      const std::string name = "JCandleSticks"); 
            
        void computeIndicator();
    };
}



