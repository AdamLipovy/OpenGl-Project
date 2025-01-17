#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include <map>

namespace QOL{
    double roundClossest(double val, double mult);

    template<typename T>
    T basicTimer(double perc, T start, T end);

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
