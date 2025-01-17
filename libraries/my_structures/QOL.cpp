
#include "QOL.hpp"

namespace QOL{
    double roundClossest(double val, double mult){

        double multOf = val / mult;
        double rest = multOf - (int)multOf;

        return mult * ((int)multOf + (rest > 0.5 ? 1 : 0));
    }


    template<typename T>
    T basicTimer(double perc, T start, T end){
        return (start - end) * (float)perc;
    }

    template <typename T>
    void ChangeBufferSubData(FunctionType<T, SubBufferType> data){
        glNamedBufferSubData(data.args.buffer, data.args.index, data.args.size, data.data);
    }
}