
#pragma once

#include "Enums.hpp"
#include <iostream>

// git - https://github.com/nlohmann/json.git
#include "../json/json.hpp"

using tile_json_data = nlohmann::json_abi_v3_11_3::json;

class GameTileData{
public:
    // six bits each containing if side has water/rail exit
    int8_t water_exit = 0;
    int8_t rail_exit = 0;
    int8_t target = 0;
    int8_t add_score = 0;
    double rotation = 0;

    TerrainTypes flag = NONE;
    TerrainTypes target_type = NONE;
    TerrainTypes hex_triangles[6];

    void setup(tile_json_data);
    void debug_print();

    bool operator==(GameTileData tile);
};