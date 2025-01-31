
#pragma once

#include <cstdlib>
#include <algorithm>
#include "QOL.hpp"

//TODO make it time based not frame based

template <typename T, typename Args>
class Timer{
private:
    size_t time_left;
    size_t max_size;
    double(*f)(float);
    T(*f_change)(double, T, T);
    T start;
    T stop;
    Args args;
    void(*adress)(QOL::FunctionType<T, Args>) = nullptr;

    double clamp(double perc){
        if(perc >= 1){ return 1; }
        else if(perc <= 0){ return 0; }
        return perc;
    }

public:
    /* @note Timer class that makes smooth transition between two edges*/
    /* @param frames_till_change number of frames that transition should take*/
    /* @param func function that takes size_t and returns double - percentige of progress in transition should be in range (0, 100)*/
    /* @param zero_val starting point*/
    /* @param one_val end point*/
    Timer(size_t frames_till_change, double(*func)(float), T(*f_change)(double, T, T), T zero_val, T hundred_val, int delay = 0){
        time_left = frames_till_change + delay;
        max_size = frames_till_change;
        f = func;
        this->f_change = f_change;
        start = zero_val;
        stop = hundred_val;
    }

    /* @note returns next iteration between start and stop*/
    T next_value(size_t change_by = 1){
        if(time_left != 0) { time_left -= change_by; }
        if (adress == nullptr){
            return f_change(Timer::clamp(f(std::max((double)(max_size - time_left), 0.0) / max_size)), start, stop);
        }
        T temp = f_change(Timer::clamp(f(std::max((double)(max_size - time_left), 0.0) / max_size)), start, stop);

        adress(QOL::FunctionType(temp, args));
        return temp;
    }

    T cur_value(){
        return f_change(Timer::clamp(f(std::max((double)(max_size - time_left), 0.0) / max_size)), start, stop);
    }

    void reconfig(size_t frames_till_change) { time_left = frames_till_change; }
    void reconfig(double(*func)(size_t)) { f = func; }
    void reconfig(T(*f_change)(double, T, T)) { this->f_change = f_change; }
    void changeStart(T zero_val) { start = zero_val; }
    void changeEnd(T hundred_val) { stop = hundred_val; }
    void changeCurEnd(T hundred_val, bool time_reset = true) {
        if(stop == hundred_val) { return; }
        start = f_change(Timer::clamp(f(std::max((double)(max_size - time_left), 0.0) / max_size)), start, stop);
        stop = hundred_val;
        if(time_reset) {time_left = max_size;}
    }
    void directChange(Args parameter, void(*direct_change)(QOL::FunctionType<T, Args>)){
        args = parameter;
        adress = direct_change;
    }

    bool is_active() { return time_left > 0; }
};