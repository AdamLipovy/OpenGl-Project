
#pragma once

#include "../SimplexNoise/SimplexNoise.h"
#include <random>

struct PerlinNoiseHandler{
protected:
    SimplexNoise sn;
    int seed;
    int(*bucket_f)(float);

public:
    PerlinNoiseHandler(int, int(*f)(float));

    virtual int get(float x, float y);
};

class OctavedPN : public PerlinNoiseHandler{
private:
    int32_t octaves;

public:
    OctavedPN(int, int(*f)(float), int32_t octaves);

    int get(float x, float y);
};

class NonOctavedPN : public PerlinNoiseHandler{
public:
    NonOctavedPN(int, int(*f)(float));

    int get(float x, float y);
};

class Randomiser{
private:
    int seed;

public:
    Randomiser();

    PerlinNoiseHandler new_perlin(int, int (*f)(float), int32_t octaves);
    
    int get_seed();
};
