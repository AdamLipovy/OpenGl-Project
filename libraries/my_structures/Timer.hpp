
#pragma once

#include <cstdlib>

template <typename T>
class Timer{
private:
    size_t time_left;
    size_t max_size;
    double(*f)(size_t);
    T(*f_change)(double, T, T);
    T start;
    T stop;
    T* adress;

    double clamp(double perc);

public:
    /* @note Timer class that makes smooth transition between two edges*/
    /* @param frames_till_change number of frames that transition should take*/
    /* @param func function that takes size_t and returns double - percentige of progress in transition should be in range (0, 100)*/
    /* @param zero_val starting point*/
    /* @param one_val end point*/
    Timer(size_t frames_till_change, double(*func)(size_t), T(*f_change)(double, T, T), T zero_val, T hundred_val, T* direct_change);

    /* @note returns next iteration between start and stop*/
    T next_value();
};