#include "application.hpp"

#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using std::make_shared;

GLuint load_texture_2d(const std::filesystem::path filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename.generic_string().data(), &width, &height, &channels, 4);

    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);

    glTextureStorage2D(texture, (GLsizei) std::log2(width), GL_RGBA8, width, height);

    glTextureSubImage2D(texture,
                        0,                         //
                        0, 0,                      //
                        width, height,             //
                        GL_RGBA, GL_UNSIGNED_BYTE, //
                        data);

    stbi_image_free(data);

    glGenerateTextureMipmap(texture);

    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments)
    : PV112Application(initial_width, initial_height, arguments) {
    this->width = initial_width;
    this->height = initial_height;

    gm_controller->SpaceToVec(glm::vec3(0.873f, 0, 5.32f));
    gm_controller->SpaceToVec(glm::vec3(-0.873f, 0, -5.32f));

    // Initializing game controller for operating
    hex_tile_storage = new HexTileUBO[gm_controller->tiles_count];
    position_storage = new glm::vec4[gm_controller->tiles_count];

    // Initializing buffers
    glCreateBuffers(1, &tile_data);
    glCreateBuffers(1, &tile_position_buffer);
    glCreateBuffers(1, &hexagon_vertexes);
    glCreateBuffers(1, &color_buffer);
    glCreateBuffers(1, &hexagon_normal_vertexes);
    glCreateVertexArrays(1, &hexagon_pos_indexed_vao);

    glNamedBufferStorage(color_buffer, sizeof(terrain_color), terrain_color, GL_DYNAMIC_STORAGE_BIT);

    glNamedBufferStorage(hexagon_vertexes, sizeof(hex_vertex), hex_vertex, 0);
    glNamedBufferStorage(hexagon_normal_vertexes, sizeof(hex_normals), hex_normals, 0);

    glVertexArrayVertexBuffer(hexagon_pos_indexed_vao, 0, hexagon_vertexes, 0, 3 * sizeof(float));

    glEnableVertexArrayAttrib(hexagon_pos_indexed_vao, 0);
    glVertexArrayAttribFormat(hexagon_pos_indexed_vao, 0, 3, GL_FLOAT, GL_TRUE, 0);
    glVertexArrayAttribBinding(hexagon_pos_indexed_vao, 0, 0);

    glVertexArrayVertexBuffer(hexagon_pos_indexed_vao, 1, hexagon_normal_vertexes, 0, 3 * sizeof(float));

    glEnableVertexArrayAttrib(hexagon_pos_indexed_vao, 1);
    glVertexArrayAttribFormat(hexagon_pos_indexed_vao, 1, 3, GL_FLOAT, GL_TRUE, 0);
    glVertexArrayAttribBinding(hexagon_pos_indexed_vao, 1, 1);

    // --------------------------------------------------------------------------
    // Initialize UBO Data
    // --------------------------------------------------------------------------
    camera_ubo.position = glm::vec4(camera.get_eye_position(), 1.0f);
    camera_ubo.projection = glm::perspective(glm::radians(45.0f), float(width) / float(height), 0.01f, 100.0f);
    camera_ubo.view = glm::lookAt(camera.get_eye_position(), camera.look_at, glm::vec3(0.0f, 1.0f, 0.0f));

    light_ubo.position = glm::vec4(0.0f, 3.0f, 0.0f, 1.0f);
    light_ubo.ambient_color = glm::vec4(0.2f);
    light_ubo.diffuse_color = glm::vec4(0.5f);
    light_ubo.specular_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    objects_ubos.push_back({.model_matrix = glm::mat4(1.0f),
                            .ambient_color = glm::vec4(0.0f),
                            .diffuse_color = glm::vec4(1.0f),
                            .specular_color = glm::vec4(0.5f)});


    // // --------------------------------------------------------------------------
    // // Create Buffers
    // // --------------------------------------------------------------------------

    glCreateBuffers(1, &camera_buffer);
    glNamedBufferStorage(camera_buffer, sizeof(CameraUBO), &camera_ubo, GL_DYNAMIC_STORAGE_BIT);

    glCreateBuffers(1, &light_buffer);
    glNamedBufferStorage(light_buffer, sizeof(LightUBO), &light_ubo, GL_DYNAMIC_STORAGE_BIT);

    glCreateBuffers(1, &objects_buffer);
    glNamedBufferStorage(objects_buffer, sizeof(ObjectUBO), objects_ubos.data(), GL_DYNAMIC_STORAGE_BIT);

    glm::ivec3 temp = glm::ivec3(0, 0, 0);

    gm_controller->play(temp);

    position_storage[0] = glm::vec4(0, 0, 0, 0);
    hex_tile_storage[0] = HexTileUBO();

    GameTileData* first_tile = nullptr;
    gm_controller->game_map.get_data(temp, &first_tile); 

    hex_tile_storage[0].position = glm::vec4(0, 0, 0, 1);

    hex_tile_storage[0].rotation = (int)first_tile->rotation;
    hex_tile_storage[0].target = first_tile->target;

    hex_tile_storage[0].triangle1 = first_tile->hex_triangles[0];
    hex_tile_storage[0].triangle2 = first_tile->hex_triangles[1];
    hex_tile_storage[0].triangle3 = first_tile->hex_triangles[2];
    hex_tile_storage[0].triangle4 = first_tile->hex_triangles[3];
    hex_tile_storage[0].triangle5 = first_tile->hex_triangles[4];
    hex_tile_storage[0].triangle6 = first_tile->hex_triangles[5];

    hex_tile_storage[0].river1 = glm::vec4(0);
    hex_tile_storage[0].river2 = glm::vec4(0);
    hex_tile_storage[0].river3 = glm::vec4(0);
    hex_tile_storage[0].river4 = glm::vec4(0);

    temp = glm::ivec3(0, 0, 1);

    gm_controller->play(temp);

    position_storage[1] = glm::vec4(temp, 0.0f);
    hex_tile_storage[1] = HexTileUBO();

    first_tile = nullptr;
    gm_controller->game_map.get_data(temp, &first_tile); 

    hex_tile_storage[1].position = glm::vec4(temp, 1);

    hex_tile_storage[1].rotation = (int)first_tile->rotation;
    hex_tile_storage[1].target = first_tile->target;

    hex_tile_storage[1].triangle1 = first_tile->hex_triangles[0];
    hex_tile_storage[1].triangle2 = first_tile->hex_triangles[1];
    hex_tile_storage[1].triangle3 = first_tile->hex_triangles[2];
    hex_tile_storage[1].triangle4 = first_tile->hex_triangles[3];
    hex_tile_storage[1].triangle5 = first_tile->hex_triangles[4];
    hex_tile_storage[1].triangle6 = first_tile->hex_triangles[5];

    hex_tile_storage[1].river1 = glm::vec4(0);
    hex_tile_storage[1].river2 = glm::vec4(0);
    hex_tile_storage[1].river3 = glm::vec4(0);
    hex_tile_storage[1].river4 = glm::vec4(0);

    glNamedBufferStorage(tile_position_buffer, sizeof(glm::vec3) * gm_controller->tiles_count, position_storage, GL_DYNAMIC_STORAGE_BIT);

    glNamedBufferStorage(tile_data, sizeof(HexTileUBO) * gm_controller->tiles_count, hex_tile_storage, GL_DYNAMIC_STORAGE_BIT);

    compile_shaders();
}

Application::~Application() {

    delete_shaders();

    glDeleteBuffers(1, &camera_buffer);
    glDeleteBuffers(1, &light_buffer);
    glDeleteBuffers(1, &objects_buffer);

    glDeleteBuffers(1, &tile_data);
    glDeleteBuffers(1, &tile_position_buffer);
    glDeleteBuffers(1, &hexagon_vertexes);
    glDeleteBuffers(1, &hexagon_normal_vertexes);
    glDeleteBuffers(1, &color_buffer);
    glDeleteVertexArrays(1, &hexagon_pos_indexed_vao);

    free(hex_tile_storage);
    free(position_storage);
}

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

void Application::delete_shaders() {}

void Application::compile_shaders() {
    delete_shaders();
    main_program = create_program(lecture_shaders_path / "main.vert", lecture_shaders_path / "main.frag");
}

void Application::update(float delta) {}

void Application::render() {
    // --------------------------------------------------------------------------
    // Update UBOs
    // --------------------------------------------------------------------------
    // Camera

    camera_ubo.position = glm::vec4(camera.get_eye_position(), 1.0f);
    camera_ubo.projection = glm::perspective(glm::radians(45.0f), static_cast<float>(width * 0.75f) / static_cast<float>(height), 0.01f, 1000.0f);
    camera_ubo.view = glm::lookAt(camera.get_eye_position(), camera.look_at, glm::vec3(0.0f, 1.0f, 0.0f));
    glNamedBufferSubData(camera_buffer, 0, sizeof(CameraUBO), &camera_ubo);

    // --------------------------------------------------------------------------
    // Draw scene
    // --------------------------------------------------------------------------
    glViewport(0, 0, (GLsizei)(width * 0.75f), (GLsizei)height);

    // Clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Configure fixed function pipeline
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Draw objects
    glUseProgram(main_program);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, camera_buffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, light_buffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, objects_buffer);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, tile_data);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, tile_position_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, color_buffer);

    glBindVertexArray(hexagon_pos_indexed_vao);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 54, gm_controller->turn);
}

void Application::render_ui() { const float unit = ImGui::GetFontSize(); }

// ----------------------------------------------------------------------------
// Input Events
// ----------------------------------------------------------------------------

void Application::on_resize(int width, int height) {
    // Calls the default implementation to set the class variables.
    PV112Application::on_resize(width, height);
}

void Application::on_mouse_move(double x, double y) { 
    camera.on_mouse_move(x, y); 
}
void Application::on_mouse_button(int button, int action, int mods) {
    if (action == GLFW_RELEASE) {
        AddTile();
    }
    camera.on_mouse_button(button, action, mods);
}

glm::vec3 Application::RayCast(){

    glm::vec3 aux = glm::vec3(0.0f);
    double RCx = 0.0;
    double RCy = 0.0;
    glfwGetCursorPos(window, &RCx, &RCy);

    float x = (2.0f * RCx) / width * 0.75 - 1.0f;
    if(x > 1 || x < -1) { return aux; }
    float y = 1.0f - (2.0f * RCy) / height;
    if(y > 1 || y < -1) { return aux; }
    float z = 1.0f;

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(width * 0.75f) / static_cast<float>(height), 0.01f, 1000.0f);

    glm::vec4 viewport = glm::vec4(0.0f, 0.0f, height, width * 0.75f);
    glm::vec3 start = glm::unProject(glm::vec3(x, y, 0.0f), camera.get_view_matrix(), projection, viewport);
    glm::vec3 end = glm::unProject(glm::vec3(x, y, 1.0f), camera.get_view_matrix(), projection, viewport);

    glm::vec3 direction = start - end;
    glm::vec3 normdirection = normalize(direction);

    float mult = (start.y / normdirection.y);

    glm::vec3 sub = normdirection * mult;

    aux = start - sub;

    glm::vec4 ray_clip = glm::vec4(x, y, -1.0, 1.0);

    glm::vec4 ray_eye = glm::inverse(camera_ubo.projection) * ray_clip;

    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

    glm::vec3 ray_wor = glm::vec3((glm::inverse(camera_ubo.view) * ray_eye));

    ray_wor = glm::normalize(ray_wor);

    glm::vec3 camera_pos = camera.get_eye_position();

    float mult_by = (camera_pos.y / -ray_wor.y);

    aux = camera_pos + ray_wor * mult_by;

    std::cout << std::format("mult: {} vec: ({}, {}, {}) camera_pos: ({}, {}, {}) aux: ({}, {}, {})\n", mult_by, ray_wor.x, ray_wor.y, ray_wor.z, camera_pos.x, camera_pos.y, camera_pos.z, aux.x, aux.y, aux.z);

    return aux;
}

void Application::on_key_pressed(int key, int scancode, int action, int mods) {
    // Calls default implementation that invokes compile_shaders when 'R key is hit.
    PV112Application::on_key_pressed(key, scancode, action, mods);
    camera.move(key, action);
}
// MY FUNCTIONS

glm::vec3 Application::GetMousePos()
{
    // Get cursor coordinates relative to the window's top-left corner.
    double relativeCursorX = 0.0;
    double relativeCursorZ = 0.0;
    glfwGetCursorPos(window, &relativeCursorX, &relativeCursorZ);

    // Get the coordinates of the window's top-left corner (relative to the top-left of the screen).
    int windowTopLeftX = 0;
    int windowTopLeftZ = 0;
    glfwGetWindowPos(window, &windowTopLeftX, &windowTopLeftZ);

    // Get the absolute coordinates of the cursor by combining the window and relative cursor coordinates.
    const int absoluteCursorX = windowTopLeftX + static_cast<int>(std::floor(relativeCursorX));
    const int absoluteCursorZ = windowTopLeftZ + static_cast<int>(std::floor(relativeCursorZ));    

    std::cout << std::format("({}, {}, {})\n", relativeCursorX, 0, relativeCursorZ);

    return glm::vec3(absoluteCursorX, 0, absoluteCursorZ);
}

void Application::AddTile(){
    glm::vec3 mousePos = RayCast();

    glm::ivec3 tilePosition = gm_controller->SpaceToVec(mousePos);
    std::cout << std::format("({}, {}, {})\n", tilePosition.x, tilePosition.y, tilePosition.z);
}

