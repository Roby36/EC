
#include "IndicatorBar.h"

/******* INDICATOR BARS ********/

namespace IndicatorBars
{
    class RSI : public IndicatorBar
    {
        public:
        double avgUp = 0, avgDown = 0, change = 0, RSI = 0;

        bool isPresent() { return (RSI != 0); }
        std::string toString() { return std::to_string(RSI); }
        std::string logString() {
            return "RSI " + std::to_string(RSI);
        }

    };

    class LocalStat : public IndicatorBar
    {
        public:
        int leftDepth = 0, rightDepth = 0;
        double leftChange = 0.0, rightChange = 0.0;
        int m = 0;

        bool isPresent() { return (m != 0); }
        std::string toString() { return std::to_string(isPresent()); }
        std::string logString() {
            if (m == -1) return "Local Maximum";
            if (m == 1)  return "Local Minimum";
            return "";
        }
    };

    class Divergence : public IndicatorBar
    {
        public:

        MBar* leftBar;
        MBar* rightBar;
        int leftBarIndex;
        int rightBarIndex;
        int divPoints = 1;
        int m = 0;
        int isDenied = false;

        bool isPresent() { return (m != 0); }
        std::string toString() { return std::to_string(this->divPoints); }
        std::string logString() {
            return ("Divergence of degree " + std::to_string(divPoints - 1) +
                    " between "             + std::string(leftBar->date_time_str) +
                    " and "                 + std::string(rightBar->date_time_str));
        }
    };

    class BollingerBands : public IndicatorBar
    {
        public:

        double bollMiddle = 0, bollUpper = 0, bollLower = 0;

        bool crossUpperUp = false, crossUpperDown = false;
        bool crossMiddleUp = false, crossMiddleDown = false;
        bool crossLowerUp = false, crossLowerDown = false;

        bool isPresent() { return bollMiddle != 0; }
        std::string toString() { 
            return std::to_string(bollLower) + " "
                 + std::to_string(bollMiddle) + " "
                 + std::to_string(bollUpper); 
        }
        std::string logString() {
            return "Lower Middle Upper Bollinger Bands: " + this->toString();
        }
    };

    class JCandleSticks : public IndicatorBar
    {
        public:

        bool bullEngulf = false, bearEngulf = false;
        bool bullHarami = false, bearHarami = false;
        bool piercing = false, darkCloud = false;
        bool morningStar = false, eveningStar = false;
        bool hammer = false;
        bool dfDoji = false, llDoji = false, gsDoji = false;

        bool isPresent() { 
            return bullEngulf || bearEngulf || bullHarami || bearHarami
                   || piercing || darkCloud || morningStar || eveningStar 
                   || hammer || dfDoji || llDoji || gsDoji; 
        }
        std::string toString() { 
            return std::to_string((int)bullEngulf)  + " "
                 + std::to_string((int)bearEngulf)  + " "
                 + std::to_string((int)bullHarami)  + " "
                 + std::to_string((int)bearHarami)  + " "
                 + std::to_string((int)piercing)    + " "
                 + std::to_string((int)darkCloud)   + " "
                 + std::to_string((int)morningStar) + " "
                 + std::to_string((int)eveningStar) + " "
                 + std::to_string((int)hammer)      + " "
                 + std::to_string((int)dfDoji)      + " "
                 + std::to_string((int)llDoji)      + " "
                 + std::to_string((int)gsDoji)      + " "; 
        }
        std::string logString() {
            std::string s = "";
            if (bullEngulf)  { s+= "Bull Engulfment; "; }
            if (bearEngulf)  { s+= "Bear Engulfment; "; }
            if (bullHarami)  { s+= "Bull Harami; "; }
            if (bearHarami)  { s+= "Bear Harami; "; }
            if (piercing)    { s+= "Piercing; "; }
            if (darkCloud)   { s+= "Dark Cloud; "; }
            if (morningStar) { s+= "Morning Star; "; }
            if (eveningStar) { s+= "Evening Star; "; }
            if (hammer)      { s+= "Hammer; "; }
            if (dfDoji)      { s+= "Dragon-fly Doji; "; }
            if (llDoji)      { s+= "Long-legged Doji; "; }
            if (gsDoji)      { s+= "Gravestone Doji; "; }
            return s;
        }
    };
};