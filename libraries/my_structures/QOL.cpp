
#include "QOL.hpp"

namespace QOL{
    float pythDist(glm::vec3 a, glm::vec3 b){
        float one = a.x - b.x;
        float two = a.z - b.z;
        return (one * one + two * two);
    }

    double roundClossest(double val, double mult){

        double multOf = val / mult;
        double rest = multOf - (int)multOf;

        if(rest < 0) {
            return mult * ((int)multOf - (rest < -0.5 ? 1 : 0));
        }
        return mult * ((int)multOf + (rest > 0.5 ? 1 : 0));
    }

    // TODO fix it somehow
    float hexagonRotation(double perc, float start, float end){
        if(start - end > 3.0f){
            start += 6;
            return start + (end - start) * (float)perc;
        }
        return start + (end - start) * (float)perc;
    }

    MatChange matTransition(double perc, MatChange start, MatChange end){
        MatChange aux = MatChange();
        aux.position = basicTimer<glm::vec3>(perc, start.position, end.position);
        aux.rotate = basicTimer<float>(perc, start.rotate, end.rotate);
        aux.rotate_around = basicTimer<glm::vec3>(perc, start.rotate_around, end.rotate_around);
        aux.size = basicTimer<glm::vec3>(perc, start.size, end.size);
        return aux;
    }
}

namespace animation_functions{
    double ease_in_ease_out(float t){
        if(t <= 0.5f) return 2.0f * t * t;
        t -= 0.5f;
        return 2.0f * t * (1.0f - t) + 0.5f;
    }
}