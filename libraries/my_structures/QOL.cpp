
#include "QOL.hpp"

namespace QOL{
    double roundClossest(double val, double mult){

        double multOf = val / mult;
        double rest = multOf - (int)multOf;

        return mult * ((int)multOf + (rest > 0.5 ? 1 : 0));
    }
}

namespace animation_functions{
    double ease_in_ease_out(float t){
        if(t <= 0.5f) return 2.0f * t * t;
        t -= 0.5f;
        return 2.0f * t * (1.0f - t) + 0.5f;
    }
}