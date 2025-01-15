// ################################################################################
// Common Framework for Computer Graphics Courses at FI MUNI.
//
// Copyright (c) Visitlab (https://visitlab.fi.muni.cz)
// All rights reserved.
// ################################################################################

#pragma once

#include "camera.h"
#include "cube.hpp"
#include "pv112_application.hpp"
#include "sphere.hpp"
#include "teapot.hpp"
#include <vector>

#include "libraries/my_structures/GameController.hpp"
#include "libraries/my_structures/MovingCamera.hpp"
#include "libraries/my_structures/QOL.hpp"

#include "math.h"

#define sqrt3fourth 0.433f

// ----------------------------------------------------------------------------
// UNIFORM STRUCTS
// ----------------------------------------------------------------------------
struct CameraUBO {
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec4 position;
};

struct LightUBO {
    glm::vec4 position;
    glm::vec4 ambient_color;
    glm::vec4 diffuse_color;
    glm::vec4 specular_color;
};

struct alignas(256) ObjectUBO {
    glm::mat4 model_matrix;  // [  0 -  64) bytes
    glm::vec4 ambient_color; // [ 64 -  80) bytes
    glm::vec4 diffuse_color; // [ 80 -  96) bytes

    // Contains shininess in .w element
    glm::vec4 specular_color; // [ 96 - 112) bytes
};

struct HexTileUBO {
public:
    glm::vec4 position;
    // first is rotation
    // second is for target
    // third to eight is target_type equal to index in terrain color matrix
    int rotation;
    int target;
    int triangle1;
    int triangle2;
    int triangle3;
    int triangle4;
    int triangle5;
    int triangle6;

    glm::vec4 river1;
    glm::vec4 river2;
    glm::vec4 river3;
    glm::vec4 river4;
};

struct ActiveHexTileUBO : public HexTileUBO{
public:
    // data of current status of animation
    glm::vec4 cur_position;
    float rotation;
};

// Constants
const float clear_color[4] = {0.0, 0.0, 0.0, 1.0};
const float clear_depth[1] = {1.0};

class Application : public PV112Application {

    // game controller
    GameController* gm_controller = new GameController();

    // @note data of position and color
    GLuint tile_data = 0;
    GLuint tile_position_buffer = 0;
    HexTileUBO* hex_tile_storage;
    glm::vec4* position_storage;

    GLuint hexagon_vertexes = 0;
    GLuint hexagon_normal_vertexes = 0;

    // @note number of tiles in buffer
    size_t tile_count = 0;

    // @note hexagon indexeded array
    GLuint hexagon_pos_indexed_vao = 0;

    GLuint color_buffer = 0;

    // ----------------------------------------------------------------------------
    // Variables
    // ----------------------------------------------------------------------------
    std::filesystem::path images_path;
    std::filesystem::path objects_path;

    // Main program
    GLuint main_program;
    // TODO: feel free to add as many as you need/like

    // List of geometries used in the project
    std::vector<std::shared_ptr<Geometry>> geometries;
    // Shared pointers are pointers that automatically count how many times they are used. When there are 0 pointers to the object pointed
    // by shared_ptrs, the object is automatically deallocated. Consequently, we gain 3 main properties:
    // 1. Objects are not unnecessarily copied
    // 2. We don't have to track pointers
    // 3. We don't have to deallocate these geometries
    // std::shared_ptr<Geometry> sphere;
    // std::shared_ptr<Geometry> bunny;

    // Default camera that rotates around center.
    MovingCamera camera = MovingCamera(false);          

    // UBOs
    GLuint camera_buffer = 0;
    CameraUBO camera_ubo;

    GLuint light_buffer = 0;
    LightUBO light_ubo;

    GLuint objects_buffer = 0;
    std::vector<ObjectUBO> objects_ubos;

    // Lights
    std::vector<LightUBO> lights;
    GLuint lights_buffer = 0;

    // Textures
    GLuint marble_texture = 0;

    // ----------------------------------------------------------------------------
    // Constructors & Destructors
    // ----------------------------------------------------------------------------
  public:
    /**
     * Constructs a new @link Application with a custom width and height.
     *
     * @param 	initial_width 	The initial width of the window.
     * @param 	initial_height	The initial height of the window.
     * @param 	arguments	  	The command line arguments used to obtain the application directory.
     */
    Application(int initial_width, int initial_height, std::vector<std::string> arguments = {});

    /** Destroys the {@link Application} and releases the allocated resources. */
    ~Application() override;

    // ----------------------------------------------------------------------------
    // Methods
    // ----------------------------------------------------------------------------

    /** @copydoc PV112Application::compile_shaders */
    void compile_shaders() override;

    /** @copydoc PV112Application::delete_shaders */
    void delete_shaders() override;

    /** @copydoc PV112Application::update */
    void update(float delta) override;

    /** @copydoc PV112Application::render */
    void render() override;

    /** @copydoc PV112Application::render_ui */
    void render_ui() override;

    // ----------------------------------------------------------------------------
    // Input Events
    // ----------------------------------------------------------------------------

    /** @copydoc PV112Application::on_resize */
    void on_resize(int width, int height) override;

    /** @copydoc PV112Application::on_mouse_move */
    void on_mouse_move(double x, double y) override;

    /** @copydoc PV112Application::on_mouse_button */
    void on_mouse_button(int button, int action, int mods) override;

    /** @copydoc PV112Application::on_key_pressed */
    void on_key_pressed(int key, int scancode, int action, int mods) override;

    // MY FUNCTIONS
    /** raycast function for distingushing where player wants to points */

private:
    glm::vec3 RayCast();
    // @ref https://www.reddit.com/r/opengl/comments/u4wcm3/comment/i4yewfh/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
    glm::vec3 GetMousePos();

    void AddTile();
};

const glm::vec4 terrain_color[] = {
    glm::vec4(0.17f, 0.92f, 0.31f, 1.0f), // MEADOW
    glm::vec4(0.91f, 0.90f, 0.0f, 1.0f), // FIELD
    glm::vec4(0.15f, 0.57f, 0.22f, 1.0f), // FOREST
    glm::vec4(0.74f, 0.73f, 0.54f, 1.0f), // CITY
    glm::vec4(0.75f, 0.75f, 0.75f, 1.0f), // RAIL
    glm::vec4(0.56f, 0.82f, 0.84f, 1.0f), // WATER
    glm::vec4(0.35f, 0.35f, 0.35f, 1.0f), // NONE
};

const float hex_vertex[] = {
    0.5f, 0.1f, 0.0f,       0.0f, 0.1f, -0.0f,      0.25f, 0.1f, sqrt3fourth,
    0.25f, 0.1f, sqrt3fourth,    0.0f, 0.1f, -0.0f,      -0.25f, 0.1f, sqrt3fourth,
    -0.25f, 0.1f, sqrt3fourth,   0.0f, 0.1f, -0.0f,      -0.5f, 0.1f, 0.0f,
    -0.5f, 0.1f, 0.0f,      0.0f, 0.1f, -0.0f,      -0.25f, 0.1f, -sqrt3fourth,
    -0.25f, 0.1f, -sqrt3fourth,  0.0f, 0.1f, -0.0f,      0.25f, 0.1f, -sqrt3fourth,
    0.25f, 0.1f, -sqrt3fourth,   0.0f, 0.1f, -0.0f,      0.5f, 0.1f, 0.0f,
    // sides
    0.5f, 0.0f, 0.0f,       0.5f, 0.1f, 0.0f,       0.25f, 0.0f, sqrt3fourth,
    0.5f, 0.1f, 0.0f,       0.25f, 0.1f, sqrt3fourth,    0.25f, 0.0f, sqrt3fourth,
    0.25f, 0.0f, sqrt3fourth,    0.25f, 0.1f, sqrt3fourth,    -0.25f, 0.0f, sqrt3fourth,
    0.25f, 0.1f, sqrt3fourth,    -0.25f, 0.1f, sqrt3fourth,   -0.25f, 0.0f, sqrt3fourth,
    -0.25f, 0.0f, sqrt3fourth,   -0.25f, 0.1f, sqrt3fourth,   -0.5f, 0.0f, 0.0f,
    -0.25f, 0.1f, sqrt3fourth,   -0.5f, 0.1f, 0.0f,      -0.5f, 0.0f, 0.0f,
    -0.5f, 0.0f, 0.0f,      -0.5f, 0.1f, 0.0f,      -0.25f, 0.0f, -sqrt3fourth,
    -0.5f, 0.1f, 0.0f,      -0.25f, 0.1f, -sqrt3fourth,  -0.25f, 0.0f, -sqrt3fourth,
    -0.25f, 0.0f, -sqrt3fourth,  -0.25f, 0.1f, -sqrt3fourth,  0.25f, 0.0f, -sqrt3fourth,
    -0.25f, 0.1f, -sqrt3fourth,  0.25f, 0.1f, -sqrt3fourth,   0.25f, 0.0f, -sqrt3fourth,
    0.25f, 0.0f, -sqrt3fourth,   0.25f, 0.1f, -sqrt3fourth,   0.5f, 0.0f, 0.0f,
    0.25f, 0.1f, -sqrt3fourth,   0.5f, 0.1f, 0.0f,       0.5f, 0.0f, 0.0f,
};

const float hex_normals[] = {
    0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f,

    sqrt3fourth, 0.0f, 0.25f,       sqrt3fourth, 0.0f, 0.25f,       sqrt3fourth, 0.0f, 0.25f,
    sqrt3fourth, 0.0f, 0.25f,       sqrt3fourth, 0.0f, 0.25f,       sqrt3fourth, 0.0f, 0.25f,
    0.0f, 0.0f, 0.5f,       0.0f, 0.0f, 0.5f,       0.0f, 0.0f, 0.5f,
    0.0f, 0.0f, 0.5f,       0.0f, 0.0f, 0.5f,       0.0f, 0.0f, 0.5f,
    -sqrt3fourth, 0.0f, 0.25f,       -sqrt3fourth, 0.0f, 0.25f,       -sqrt3fourth, 0.0f, 0.25f,
    -sqrt3fourth, 0.0f, 0.25f,       -sqrt3fourth, 0.0f, 0.25f,       -sqrt3fourth, 0.0f, 0.25f,
    -sqrt3fourth, 0.0f, -0.25f,       -sqrt3fourth, 0.0f, -0.25f,       -sqrt3fourth, 0.0f, -0.25f,
    -sqrt3fourth, 0.0f, -0.25f,       -sqrt3fourth, 0.0f, -0.25f,       -sqrt3fourth, 0.0f, -0.25f,
    0.0f, 0.0f, -0.5f,       0.0f, 0.0f, -0.5f,       0.0f, 0.0f, -0.5f,
    0.0f, 0.0f, -0.5f,       0.0f, 0.0f, -0.5f,       0.0f, 0.0f, -0.5f,
    sqrt3fourth, 0.0f, -0.25f,       sqrt3fourth, 0.0f, -0.25f,       sqrt3fourth, 0.0f, -0.25f,
    sqrt3fourth, 0.0f, -0.25f,       sqrt3fourth, 0.0f, -0.25f,       sqrt3fourth, 0.0f, -0.25f,
};