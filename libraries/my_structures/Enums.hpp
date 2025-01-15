
#pragma once

#include <cstdlib>
#include <cstdint>
#include <cmath>

#ifndef SQRTDIST
const double CENTER_DIST = std::sqrt(3)/2; // precalculating the value to save time
#define SQRTDIST CENTER_DIST
#endif

enum TerrainTypes{
    NONE,       // 0
    MEADOW,     // 1
    FIELD,      // 2
    FOREST,     // 3
    CITY,       // 4
    RAIL,       // 5
    WATER       // 6
};

enum GameStatus{
    NORMAL,
    TARGET,
    NO_MOVE,
    END,
    ERROR
};