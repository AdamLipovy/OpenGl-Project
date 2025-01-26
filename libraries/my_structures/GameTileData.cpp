#include "GameTileData.hpp"

void printEnumToString(TerrainTypes t)
{
    switch (t)
    {
    case NONE:
        std::cout << "NONE";
        return;
    case MEADOW:
        std::cout << "MEADOW";
        return;
    case FIELD:
        std::cout << "FIELD";
        return;
    case FOREST:
        std::cout << "FOREST";
        return;
    case CITY:
        std::cout << "CITY";
        return;
    }
}

void printEnumToString(GameStatus t)
{
    switch (t)
    {
    case NORMAL:
        std::cout << "NORMAL";
        return;
    case TARGET:
        std::cout << "TARGET";
        return;
    case NO_MOVE:
        std::cout << "NO_MOVE";
        return;
    case END:
        std::cout << "END";
        return;
    case ERROR:
        std::cout << "ERROR";
        return;
    }
}

bool GameTileData::operator==(GameTileData tile){
    for(int i = 0; i < 6; i++){
        if(hex_triangles[i] != tile.hex_triangles[i]) { return false; }
    }
    return water_exit == tile.water_exit && rail_exit == tile.rail_exit &&
            target == tile.target && add_score == tile.add_score &&
            rotation == tile.rotation && flag == tile.flag &&
            target_type == tile.target_type;
}

// GameTileData struct

/* @note takes one tile data from json and sets up whole GameTileData structure*/
/* @param tile_json_data data */
void GameTileData::setup(tile_json_data data)
{
    water_exit = data["water"];
    rail_exit = data["rail"];

    target = (rand() % 3) + 3;
    target_type = data["target_type"];
    flag = data["flag"];

    for (size_t i = 0; i < 6; i++)
    {
        hex_triangles[i] = data["terrain_type"][i];
    }

    rotation = 0;
    return;
}

/* @note function for debuging purposes*/
void GameTileData::debug_print()
{
    printf("water: %d\n", water_exit);
    printf("rail: %d\n", rail_exit);

    printf("target: %d\n", target);
    printf("target type: ");
    printEnumToString(target_type);
    printf("\n");

    printf("water: %d\n", water_exit);

    std::cout << "triangles types: [";
    for (size_t i = 0; i < 6; i++)
    {
        printEnumToString(hex_triangles[i]);
        if (i != 5)
        {
            std::cout << ",";
        }
    }
    std::cout << "]" << std::endl;
}
