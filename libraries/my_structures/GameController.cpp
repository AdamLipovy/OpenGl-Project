
#include "GameController.hpp"

template <typename T>
bool contains(std::vector<T> vec, T el)
{
    for(size_t i = 0; i < vec.size(); i++)
    {
        if(vec[i] == el) { return true; }
    }
    return false;
}

size_t GameController::flood_check(glm::ivec3 origin, TerrainTypes type, size_t target_count)
{
    std::deque<glm::ivec3> q;
    std::vector<glm::ivec3> seen = std::vector<glm::ivec3>();
    bool has_seen;
    int count = 1;
    q.push_back(origin);

    GameTileData *tile = nullptr;
    GameTileData *adjusted_tile = nullptr;

    glm::ivec3 adjusted_pos = glm::ivec3();

    while (q.size() != 0)
    {
        glm::ivec3 n = q.front();

        q.pop_front();
        if (!game_map.get_data(n, &tile))
        {
            continue;
        }

        int rotation = (int)round(tile->rotation);
        for (size_t i = 0; i < 6; i++)
        {
            if (tile->hex_triangles[(i + rotation) % 6] == type)
            {
                has_seen = false;
                adjusted_pos = n + OFFSETMAP[(i + rotation) % 6];

                if (contains(seen, adjusted_pos))
                {
                    continue;
                }

                if (game_map.get_data(adjusted_pos, &adjusted_tile))
                {
                    if (adjusted_tile->hex_triangles[(i + rotation - 3) % 6])
                    {
                        count++;
                        q.push_back(adjusted_pos);
                    }
                    if (count > target_count && target_count != -1)
                    {
                        return -1;
                    }
                }
            }
        }
    }
    if (target_count == -1)
    {
        return count;
    }
    return target_count == count ? 0 : 1;
}

template <typename T>
void GameController::shuffle(std::vector<T*>* list)
{
    T* el0;

    int i0;
    int i1;
    size_t size = list->size();
    for (int iteration = 0; iteration < size * 3; iteration++)
    {
        i0 = iteration % size;
        i1 = rand() % size;

        el0 = (*list)[i0];
        (*list)[i0] = (*list)[i1];
        (*list)[i1] = el0;
    }
}

GameController::GameController()
{
    srand(time(0));
    if (!load())
    {
        perror("[ERROR] Couldn't load json data - terminating\n");
        return;
    }
    status = TARGET;

    shuffle(&normal_tiles);
    shuffle(&target_tiles);

    optional_map.add(glm::ivec3(0), nullptr);

    next_turn();
}

GameController::~GameController()
{
}

bool GameController::load()
{
    std::ifstream file(GAME_DATA_FILE);
    if (!file.is_open())
    {
        perror("[ERROR] - file ");
        perror(GAME_DATA_FILE);
        perror(" could not be open - terminating\n");
        assert(false);
        return false;
    }

    json data;
    file >> data;

    if (data.empty())
    {
        status = ERROR;
        perror("[ERROR] - storage was nonexistant or empty\n");
        assert(false);
        return false;
    }

    if (!data.contains("tiles")
        || !data["tiles"].is_array()){
        status = ERROR;
        perror("[ERROR PARSING] - JSON does not contain tiles or is not an array\n");
        assert(false);
        return false;        
    }

    for (const auto& tile : data["tiles"])
    {
        GameTileData* new_tile = new GameTileData();

        new_tile->setup(tile);

        if (tile["flag"] != 0)
        {
            flag_tiles.push_back(new_tile);
        }

        if (tile["is_target"])
        {
            target_tiles.push_back(new_tile);
            continue;
        }
        normal_tiles.push_back(new_tile);
    }

    normal_tiles.shrink_to_fit();
    target_tiles.shrink_to_fit();

    normal_tiles_count = normal_tiles.size();
    target_tiles_count = target_tiles.size();

    normal_tiles_left = normal_tiles_count;
    target_tiles_left = target_tiles_count;

    for(int i = 0; i < 3; i++){
        target_data.push_back(std::tuple<GameTileData *, glm::ivec3>(nullptr, glm::ivec3(0, 0, 0)));
    }

    tiles_count = normal_tiles_count + target_tiles_count;

    status = TARGET;

    return true;
}

size_t GameController::eval()
{
    return score;
}

void GameController::next_turn(){
    if(status == TARGET){
        if(target_tiles_left != 0){
            selected_tile = target_tiles[target_tiles_count - target_tiles_left];
            target_tiles_left--;
        } else if(normal_tiles_left == 0){
            status = END;
        } else {
            status = NORMAL;
            next_turn();
        }
        return;
    } else if(status == NORMAL) {
        selected_tile = normal_tiles[normal_tiles_count - normal_tiles_left];
        normal_tiles_left--;
        return;
    }
    return;
}

void GameController::target_check()
{
    int temp = targets_active;
    for (size_t i = 0; i < temp; i++)
    {

        GameTileData *ttile = std::get<0>(target_data[i]);
        glm::ivec3 ttile_pos = std::get<1>(target_data[i]);

        int8_t target_size = ttile->target;

        if (ttile->target_type == NONE)
        {
            perror("Non-target tile is somewhere, where it shouldn't be");
            assert(false);
            return;
        }
        else
        {
            switch (flood_check(ttile_pos, ttile->target_type, ttile->target))
            {
            case -1:
                // not enough tiles around. Target stays.
                break;

            case 0:
                score += ttile->add_score;
                std::get<0>(target_data[i]) = nullptr;
                status = TARGET;
                targets_active--;
                break;

            case 1:
                std::get<0>(target_data[i]) = nullptr;
                status = TARGET;
                targets_active--;
                break;

            default:
                perror("It should not return other value than +-1 and 0 but somehow did");
                assert(false);
                return;
            }
        }
    }

    // shift to the array for indexing next time
    int setTo = 0;
    for (int i = 0; i < target_data.size(); i++)
    {
        if (std::get<0>(target_data[i]) != nullptr)
        {
            target_data[setTo] = target_data[i];
            setTo++;
        }
    }
}

void GameController::reset()
{
    //TODO

    if (!load())
    {
        perror("[ERROR] Couldn't load json data - terminating\n");
        return;
    }
    status = TARGET;

    shuffle(&normal_tiles);
    shuffle(&target_tiles);
    optional_map.add(glm::ivec3(0), nullptr);
}

size_t GameController::tile_count(){
    return game_map.get_size();
}

bool GameController::play(glm::vec3 position){
    return play(SpaceToVec(position));
}

bool GameController::play(glm::ivec3 position){
    if(game_map.contains(position) || !optional_map.contains(position)){
        return false;
    }

    game_map.add(position, selected_tile);
    if (selected_tile->target != 0){
        target_data[targets_active] = std::tuple<GameTileData*, glm::ivec3>(selected_tile, position);
        targets_active++;
    }
    for (size_t i = 0; i < 6; i++)
    {
        optional_map.add(position + OFFSETMAP[i], nullptr);
    }
    
    target_check();
    if(targets_active < 3){
        status = TARGET;
    } else {
        status = NORMAL;
    }
    next_turn();
    return true;
}

glm::vec3 GameController::VecToSpace(glm::ivec3 vec){
    return glm::vec3(SPACEX.x * vec.x + SPACEZ.x * vec.z, 0, SPACEX.z * vec.z + SPACEZ.z * vec.z);
}

glm::ivec3 GameController::SpaceToVec(glm::vec3 vec){
    glm::vec3 aux = glm::vec3(0.0f);

    glm::vec3 roundedVec = glm::vec3(QOL::roundClossest(vec.x, SQRTDIST), 0, QOL::roundClossest(vec.z, 0.5f));

    int x = (int)(roundedVec.x / SQRTDIST);
    int z = (int)(roundedVec.z / 0.5f);

    aux = glm::ivec3(0);

    std::cout << std::format("{}, {}\n", x, z);
    if(abs(x % 2) != abs(z % 2)) { 
        return aux;
    }

    aux.x = (z - x) / 2;
    aux.z = aux.x + x;

    return aux;
}
