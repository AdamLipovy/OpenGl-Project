
#pragma once

#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include <vector>
#include <math.h>
#include <map>

#include "GameTileData.hpp"

#define MAPSTORAGEY std::map<int, GameTileData*>
#define MAPSTORAGEX std::map<int, MAPSTORAGEY>

glm::vec3 to3D(glm::ivec3);

class MapManager{
private:
    int size;    
    MAPSTORAGEX storage;

    glm::ivec3 to2D(glm::ivec3);

public:

    MapManager(){
        storage = MAPSTORAGEX();
    }

    ~MapManager(){
        storage.clear();
    }

    bool add(glm::ivec3, GameTileData*);
    bool remove(glm::ivec3);
    bool contains(glm::ivec3);

    bool get_data(glm::ivec3, GameTileData**);
    int get_size() {return size;}
};
