
#pragma once

#include <cstdlib>
#include <deque>
#include <vector>
#include <fstream>

#include "Enums.hpp"
#include "GameTileData.hpp"
#include "MapManager.hpp"

#include "../json/json.hpp"

using json = nlohmann::json;

#define MAX_TARGETS_ACTIVE 3

#ifndef GAMEVECTORS
#define GAMEVECTORS
#define VECX glm::ivec3(1, 0, 0)
#define VECY glm::ivec3(1, 0, 1)
#define VECZ glm::ivec3(0, 0, 1)

#define SPACEX glm::vec3(0.5f, 0, SQRTDIST)
#define SPACEZ glm::vec3(0.5f, 0, -SQRTDIST)
#endif

#ifndef GAME_DATA_FILE
#define GAME_DATA_FILE "../data.json"
#endif

const glm::ivec3 OFFSETMAP[] = {VECX, VECY, VECZ, (-1) * VECX, (-1) * VECY, (-1) * VECZ};

class GameController{
private:
    GameStatus status = TARGET;

    size_t score = 0;

    // Size of target tiles stack
    size_t target_tiles_count;
    // Tiles left from stack
    size_t target_tiles_left;
    // Size of normal tiles stack
    size_t normal_tiles_count;
    // Normal tiles left in its stack
    size_t normal_tiles_left;

    // currently selected tile
    GameTileData* selected_tile;

    // Storage for different tiles from file

    std::vector<GameTileData*> normal_tiles = std::vector<GameTileData *>();
    std::vector<GameTileData*> target_tiles = std::vector<GameTileData *>();
    std::vector<GameTileData*> flag_tiles = std::vector<GameTileData *>();

    // current active targets and data connected with them for faster access
    std::vector<std::tuple<GameTileData*, glm::ivec3>> target_data = std::vector<std::tuple<GameTileData*, glm::ivec3>>();
    int targets_active = 0;

    std::vector<glm::ivec3>* flag_position_array;

    /* @note recursively checks surrounding tiles for same type and returns ammount of tiles, that connects via type to the origin tile*/
    /* @param origin position start tile from what flood check starts*/
    /* @param type type of terrain type that function is looking for*/
    /* @param target_count ammount of tiles after which flood fill should stop*/
    /* -1 for returning count of connected tiles*/
    /* @return - count of connected tiles if target_count == -1*/
    /* @return - -1 if count of connected tiles was less than target_count*/
    /* @return - 0 if count of connected tiles was equal to target_count*/
    /* @return - 1 if count of connected tiles was more than target_count*/
    size_t flood_check(glm::ivec3, TerrainTypes, size_t);

    /* @note shuffles items based on pseudorandom values*/
    /* @param list array that wants to be shuffled*/
    template <typename T>
    void shuffle(std::vector<T*>*);

public:
    size_t turn = 0;
    // Position storage for all tiles
    MapManager optional_map;
    MapManager game_map;

    size_t tiles_count;

    GameController();
    ~GameController();

    /* @note loads json file from GAME_DATA_FILE and initializes all tiles listed in json*/
    /* @return - true if file was successfully opened, loaded, parsed and all tiles initialized succesfully*/
    /* @return - false if an error occured*/
    bool load();

    /* @note evaluates the game at the end*/
    size_t eval();

    void next_turn();

    /* @note checks all active targets, if they were completed and new tiles should be drawn*/
    void target_check();

    /* @note resets whole game map*/
    void reset();

    size_t tile_count();

    bool play(glm::ivec3);

    static glm::vec3 VecToSpace(glm::ivec3);

    static glm::ivec3 SpaceToVec(glm::vec3);
};
