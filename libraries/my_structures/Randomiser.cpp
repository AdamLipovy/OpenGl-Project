
#include "Randomiser.hpp"

Randomiser::Randomiser(){ this->seed = rand(); }

int Randomiser::get_seed(){ return this->seed; }

PerlinNoiseHandler Randomiser::new_perlin(int seed, int(*f)(float), int32_t octaves = 0){
    if(octaves == 0){
        return NonOctavedPN(seed, f);
    }
    return OctavedPN(seed, f, octaves);
}

PerlinNoiseHandler::PerlinNoiseHandler(int seed, int(*f)(float)){
    this->seed = seed;
    this->sn = SimplexNoise();
    bucket_f = f;
}


OctavedPN::OctavedPN(int seed, int(*f)(float), int32_t octaves) : PerlinNoiseHandler(seed, f){
    this->octaves = octaves;
}

int OctavedPN::get(float x, float y){
    return bucket_f(sn.fractal(octaves, x, y, seed));
}

NonOctavedPN::NonOctavedPN(int seed, int(*f)(float)) : PerlinNoiseHandler(seed, f){}

int NonOctavedPN::get(float x, float y){
    return bucket_f(sn.noise(x, y, seed));
}