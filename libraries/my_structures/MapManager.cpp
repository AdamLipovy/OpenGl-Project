
#include "MapManager.hpp"

// MapManager struct

glm::vec3 to3D(glm::ivec3 vec){
    int temp = std::min(vec.x, vec.z);
    return glm::vec3((vec.x - temp) * SQRTDIST, temp, (vec.z - temp) * CENTER_DIST);
}

// (0, 1, 0) -> (1, 0, 1)
// (0, -1, 0) -> (-1, 0, -1)
glm::ivec3 MapManager::to2D(glm::ivec3 vec){
    return glm::ivec3(vec.x + vec.y, 0, vec.z + vec.y);
}

/* @note adds data to position in map */
/* @param pos position of data in a map */
/* @param data data that should be stored on desired position */
/* @return - true if position was not yet in the map */
/* @return - false if pos does exists in the map */
bool MapManager::add(glm::ivec3 pos, GameTileData* data)
{
    pos = to2D(pos);
    if (storage.find(pos.x) == storage.end())
    {
        MAPSTORAGEY tempY;
        tempY[pos.z] = data;
        storage[pos.x] = tempY;

        size++;
        return true;
    }

    MAPSTORAGEY* sub_map_y = &storage[pos.x];

    if (sub_map_y->find(pos.z) == sub_map_y->end())
    {
        (*sub_map_y)[pos.z] = data;

        size++;
        return true;
    }

    return false;
}

/* @note removes data by position*/
/* @param pos position of data in a map*/
/* @return - true if position was found and deleted */
/* @return - false if pos does not exists in the map */
bool MapManager::remove(glm::ivec3 pos)
{
    pos = to2D(pos);
    if (storage.find(pos.x) == storage.end())
    {
        return false;
    }

    MAPSTORAGEY* sub_map_y = &storage[pos.x];

    if (sub_map_y->find(pos.z) == sub_map_y->end())
    {
        return false;
    }

    sub_map_y->erase(pos.z);
    size--;
    if (sub_map_y->size() == 0)
    {
        storage.erase(pos.z);
    }

    return true;
}

/* @note if position is present in the map*/
/* @param pos position of data in a map*/
/* @return - true if position was found in the map*/
/* @return - false if pos was not found in the map*/
bool MapManager::contains(glm::ivec3 pos)
{
    pos = to2D(pos);
    if (storage.find(pos.x) == storage.end())
    {
        return false;
    }
    MAPSTORAGEY sub_map_y = storage[pos.x];

    if (sub_map_y.find(pos.z) == sub_map_y.end())
    {
        return false;
    }

    return true;
}

/* @note if position is present in the map*/
/* @param pos position of data in a map*/
/* @param aux space for pointer where data will be outputted*/
/* @return - true if position was found in the map and pointer successfully attached*/
/* @return - false if pos was not found in the map*/
bool MapManager::get_data(glm::ivec3 pos, GameTileData** aux)
{
    pos = to2D(pos);
    if (storage.find(pos.x) == storage.end())
    {
        return false;
    }
    MAPSTORAGEY sub_map_y = storage[pos.x];

    if (sub_map_y.find(pos.z) == sub_map_y.end())
    {
        return false;
    }

    *aux = sub_map_y[pos.z];
    return true;
}
