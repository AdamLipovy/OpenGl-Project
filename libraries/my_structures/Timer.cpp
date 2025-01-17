#include "Timer.hpp"

template<typename T, typename Args>
Timer<T, Args>::Timer(size_t frames_till_change, double(*func)(float), T(*f_change)(double, T, T), T zero_val, T hundred_val){
    time_left = frames_till_change;
    max_size = frames_till_change;
    f = func;
    this->f_change = f_change;
    start = zero_val;
    stop = hundred_val;
}

template<typename T, typename Args>
double Timer<T, Args>::clamp(double perc){
    if(perc >= 100){ return 100; }
    else if(perc <= 0){ return 0; }
    return perc;
}

template<typename T, typename Args>
void Timer<T, Args>::directChange(Args parameters, void(*direct_change)(QOL::FunctionType<T, Args>)){
    args = parameters;
    adress = direct_change;
}

template<typename T, typename Args>
T Timer<T, Args>::next_value(){
    if (adress == nullptr){
        return f_change(Timer::clamp(f((time_left--) / max_size)), start, stop);
    }
    T temp = f_change(Timer::clamp(f((time_left--) / max_size)), start, stop);
    adress(temp, args);
    return temp;
}

namespace animation_functions{
    double ease_in_ease_out(float t){
        if(t <= 0.5f) return 2.0f * t * t;
        t -= 0.5f;
        return 2.0f * t * (1.0f - t) + 0.5f;
    }
}