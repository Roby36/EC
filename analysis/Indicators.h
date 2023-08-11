
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

        RSI(Bars* dp, const std::string name = "RSI",  const int TimePeriod = 14, const int starting_bar = 1) 
            : Indicator(dp, starting_bar, name), timePeriod(TimePeriod)
        {
        }

        void computeIndicatorBar(int& d);
    };

    class LocalMin : public Indicator<IndicatorBars::LocalStat>
    {
        protected:
        int m = 1;
        int curr_it_bar;

        public:
        LocalMin(Bars* dp, const std::string name = "LocalMin", const int starting_bar = 1) 
            : Indicator(dp, starting_bar, name), curr_it_bar(starting_bar)
        {
        }

        void computeIndicatorBar(int& d) override;
        void computeIndicator() override;
    };

    class LocalMax : public LocalMin
    { 
        public:
        LocalMax(Bars* dp, const std::string name = "LocalMax",  const int starting_bar = 1) 
            : LocalMin(dp, name, starting_bar)
        { this->m = -1; }
    };

    class Divergence : public Indicator<IndicatorBars::Divergence>
    {
        protected:
        // Reference to required Indicators
        class LocalMax * const LocalMax;
        class LocalMin * const LocalMin;
        class RSI * const RSI;

        public:
        /*** (Elementary) divergence parameters ***/
        const int minDivPeriod;
        const int maxDivPeriod;

        Divergence(Bars* dp,
                class LocalMax* const LocalMax, 
                class LocalMin* const LocalMin, 
                class RSI* const RSI,
                const int minDivPeriod, 
                const int maxDivPeriod,
                const int minRSItimePeriods = 10,
                const std::string name = "Divergence")
        : Indicator(dp, minRSItimePeriods * RSI->timePeriod, name), 
           LocalMax(LocalMax), LocalMin(LocalMin), RSI(RSI),
           minDivPeriod(minDivPeriod), maxDivPeriod(maxDivPeriod)
        {
        }

        virtual void computeLocalStat(class LocalMin * const LocalStat, int& d);
        void computeIndicatorBar(int& d);
        void markDivergence(int leftBarIndex, int rightBarIndex, int m);
    };

    class LongDivergence : public Divergence
    {
        public:

        LongDivergence(Bars* dp, 
                      class LocalMax* const LocalMax, 
                      class LocalMin* const LocalMin, 
                      class RSI* const RSI,
                      int minDivPeriod,
                      int maxDivPeriod,
                      const int minRSItimePeriods = 10,
                      const std::string name = "LongDivergence")
            : Divergence(dp, LocalMax, LocalMin, RSI, minDivPeriod, maxDivPeriod, minRSItimePeriods, name)
        {
        }

        void computeLocalStat(class LocalMin * const LocalStat, int& d) override;
    };

    class BollingerBands : public Indicator<IndicatorBars::BollingerBands>
    {
        /*** Bollinger Bands parameters ***/
        const double stDevUp, stDevDown;
        const int timePeriod;

        public:

        BollingerBands(Bars* dp, 
                       const double stDevUp = 2.0, 
                       const double stDevDown = 2.0, 
                       const int timePeriod = 20,
                       const std::string name = "BB")
                : Indicator(dp, timePeriod, name), 
                  stDevUp(stDevUp), stDevDown(stDevDown), timePeriod(timePeriod) 
        {
        }
            
        void computeIndicatorBar(int& d);
    };

    class JCandleSticks : public Indicator<IndicatorBars::JCandleSticks>
    {
        /*** JCandleSticks parameters ***/
        const double hammerSize;
        const double dojiSize;
        const int dojiExtremes;

        public:

        JCandleSticks(Bars* dp,
                      const double hammerSize = 3.0, 
                      const double dojiSize = 8.0, 
                      const int dojiExtremes = 3,
                      const std::string name = "JCandleSticks",
                      const int starting_bar = 2)
                : Indicator(dp, starting_bar, name),
                 hammerSize(hammerSize), dojiSize(dojiSize), dojiExtremes(dojiExtremes)
        {
        }
 
        void computeIndicatorBar(int& d);
    };
}



