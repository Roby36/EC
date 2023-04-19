
#include "IndicatorBar.h"

#include <string>

/******* INDICATOR BARS ********/

namespace IndicatorBars
{
    class RSI : public IndicatorBar
    {
        public:
        float avgUp = 0, avgDown = 0, change = 0, RSI = 0;

        bool isPresent() { return (RSI != 0); }

        string toString() { return to_string(RSI); }

        string logString() {
            return "RSI " + to_string(RSI);
        }
    };

    class LocalStat : public IndicatorBar
    {
        public:
        int leftDepth = 0, rightDepth = 0;
        float leftChange = 0.0, rightChange = 0.0;
        int m = 0;

        bool isPresent() { return (m != 0); }

        string toString() { return to_string(isPresent()); }

        string logString() {
            if (m == -1) {
                return "Local Maximum";
            }
            if (m == 1) {
                return "Local Minimum";
            }
            return "";
        }
    };

    class Divergence : public IndicatorBar
    {
        public:

        int divPoints = 1;
        int leftPos = 0;
        int rightPos = 0;
        int m = 0;

        bool isPresent() { return (m != 0); }

        string toString() { return to_string(this->divPoints); }

        string logString() {
            return "Divergence of degree " + to_string(divPoints - 1);
        }
    };

    class BollingerBands : public IndicatorBar
    {
        public:

        float bollMiddle = 0, bollUpper = 0, bollLower = 0;

        bool crossUpperUp = false, crossUpperDown = false;
        bool crossMiddleUp = false, crossMiddleDown = false;
        bool crossLowerUp = false, crossLowerDown = false;

        bool isPresent() { return bollMiddle != 0; }

        string toString() { 
            return to_string(bollLower) + " "
                 + to_string(bollMiddle) + " "
                 + to_string(bollUpper); 
        }
        
        string logString() {
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

        string toString() { 
            return to_string((int)bullEngulf) + " "
                 + to_string((int)bearEngulf) + " "
                 + to_string((int)bullHarami) + " "
                 + to_string((int)bearHarami) + " "
                 + to_string((int)piercing) + " "
                 + to_string((int)darkCloud) + " "
                 + to_string((int)morningStar) + " "
                 + to_string((int)eveningStar) + " "
                 + to_string((int)hammer) + " "
                 + to_string((int)dfDoji) + " "
                 + to_string((int)llDoji) + " "
                 + to_string((int)gsDoji) + " "; 
        }

        string logString() {
            string s = "";
            if (bullEngulf) { s+= "Bull Engulfment; "; }
            if (bearEngulf) { s+= "Bear Engulfment; "; }
            if (bullHarami) { s+= "Bull Harami; "; }
            if (bearHarami) { s+= "Bear Harami; "; }
            if (piercing) { s+= "Piercing; "; }
            if (darkCloud) { s+= "Dark Cloud; "; }
            if (morningStar) { s+= "Morning Star; "; }
            if (eveningStar) { s+= "Evening Star; "; }
            if (hammer) { s+= "Hammer; "; }
            if (dfDoji) { s+= "Dragon-fly Doji; "; }
            if (llDoji) { s+= "Long-legged Doji; "; }
            if (gsDoji) { s+= "Gravestone Doji; "; }
            return s;
        }
    };



};