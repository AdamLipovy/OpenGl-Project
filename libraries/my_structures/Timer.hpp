
#pragma once

#include <cstdlib>
#include "QOL.hpp"

template <typename T, typename Args>
class Timer{
private:
    size_t time_left;
    size_t max_size;
    double(*f)(size_t);
    T(*f_change)(double, T, T);
    T start;
    T stop;
    Args args;
    void(*adress)(QOL::FunctionType<T, Args>) = nullptr;

    double clamp(double perc);

public:
    /* @note Timer class that makes smooth transition between two edges*/
    /* @param frames_till_change number of frames that transition should take*/
    /* @param func function that takes size_t and returns double - percentige of progress in transition should be in range (0, 100)*/
    /* @param zero_val starting point*/
    /* @param one_val end point*/
    Timer(size_t frames_till_change, double(*func)(float), T(*f_change)(double, T, T), T zero_val, T hundred_val);

    /* @note returns next iteration between start and stop*/
    T next_value();

    void reconfig(size_t frames_till_change) { time_left = frames_till_change; }
    void reconfig(double(*func)(size_t)) { f = func; }
    void reconfig(T(*f_change)(double, T, T)) { this->f_change = f_change; }
    void changeStart(T zero_val) { start = zero_val; }
    void changeEnd(T hundred_val) { stop = hundred_val; }
    void directChange(Args, void(*direct_change)(QOL::FunctionType<T, Args>));

    bool is_active() { return time_left > 0; }
};

namespace animation_functions{
    double ease_in_ease_out(float t);
}