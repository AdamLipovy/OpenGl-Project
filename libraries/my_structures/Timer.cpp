#include "Timer.hpp"

template <typename T>
Timer<T>::Timer(size_t frames_till_change, double(*func)(size_t), T(*f_change)(double, T, T), T zero_val, T hundred_val, T* direct_change = nullptr){
    time_left = frames_till_change;
    max_size = frames_till_change;
    f = func;
    this->f_change = f_change;
    start = zero_val;
    stop = hundred_val;
    adress = direct_change;
}

template <typename T>
double Timer<T>::clamp(double perc){
    if(perc >= 100){ return 100; }
    else if(perc <= 0){ return 0; }
    return perc;
}

template <typename T>
T Timer<T>::next_value(){
    if (adress == nullptr){
        return f_change(Timer::clamp(f(time_left--)), start, stop);
    }
    T temp = f_change(Timer::clamp(f(time_left--)), start, stop);
    *adress = temp;
    return temp;
}