
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include <cstdlib>

namespace QOL{
    double roundClossest(double val, double mult);

    template<typename T>
    T basicTimer(double perc, T start, T end){
        return start + (end - start) * (float)perc;
    }

    template <typename T, typename args>
    struct FunctionType {
        T data;
        args args;
    };

    struct SubBufferType{
        size_t buffer;
        size_t index;
        size_t size;
    };

    template <typename T>
    void ChangeBufferSubData(FunctionType<T, SubBufferType>);
}

namespace animation_functions{
    double ease_in_ease_out(float t);
}
