#include "application.hpp"

#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using std::make_shared;

#define visualize_height -0.1f

double temp1 (float x){return (double)x;}

float temp2 (double x, float a, float b) {return a;}

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

    images_path = lecture_folder_path / "images";
    objects_path = lecture_folder_path / "objects";

    // Initializing game controller for operating
    hex_tile_storage = new HexTileUBO[gm_controller->tiles_count];
    position_storage = new glm::vec4[gm_controller->tiles_count];
    active_tile_storage = new ActiveHexTileUBO();

    // Initializing buffers
    glCreateBuffers(1, &tile_data);
    glCreateBuffers(1, &active_tile_data);
    glCreateBuffers(1, &visualize_placement);
    glCreateBuffers(1, &selected_object_buffer);

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
    camera_ubo.projection = glm::perspective(glm::radians(45.0f), (float(width) * 0.75f) / float(height), 0.01f, 100.0f);
    camera_ubo.view = glm::lookAt(camera.get_eye_position(), camera.look_at, glm::vec3(0.0f, 1.0f, 0.0f));

    light_ubo.position = glm::vec4(0.0f, 3.0f, 0.0f, 1.0f);
    light_ubo.ambient_color = glm::vec4(0.2f);
    light_ubo.diffuse_color = glm::vec4(0.5f);
    light_ubo.specular_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    objects_ubos.push_back({.model_matrix = glm::mat4(1.0f),
                            .ambient_color = glm::vec4(0.0f),
                            .diffuse_color = glm::vec4(1.0f),
                            .specular_color = glm::vec4(0.5f)});

    // same for second camera

    selected_camera_ubo.position = glm::vec4(selected_camera.get_eye_position(), 1.0f);
    selected_camera_ubo.projection = glm::perspective(glm::radians(45.0f), (float(width) * 0.25f) / (float(height) * 0.25f), 0.01f, 100.0f);
    selected_camera_ubo.view = glm::lookAt(selected_camera.get_eye_position(), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    selected_object_ubo = new ObjectUBO{.model_matrix = glm::mat4(1.0f),
                                        .ambient_color = glm::vec4(0.0f),
                                        .diffuse_color = glm::vec4(1.0f),
                                        .specular_color = glm::vec4(0.5f)};


    // // --------------------------------------------------------------------------
    // // Create Buffers
    // // --------------------------------------------------------------------------

    glCreateBuffers(1, &camera_buffer);
    glNamedBufferStorage(camera_buffer, sizeof(CameraUBO), &camera_ubo, GL_DYNAMIC_STORAGE_BIT);

    glCreateBuffers(1, &light_buffer);
    glNamedBufferStorage(light_buffer, sizeof(LightUBO), &light_ubo, GL_DYNAMIC_STORAGE_BIT);

    glCreateBuffers(1, &objects_buffer);
    glNamedBufferStorage(objects_buffer, sizeof(ObjectUBO), objects_ubos.data(), GL_DYNAMIC_STORAGE_BIT);

    // selected camera

    glCreateBuffers(1, &selected_camera_buffer);
    glNamedBufferStorage(selected_camera_buffer, sizeof(CameraUBO), &selected_camera_ubo, GL_DYNAMIC_STORAGE_BIT);

    glm::ivec3 temp = glm::ivec3(0, 0, 0);
    gm_controller->play(temp);

    size_t index = gm_controller->tile_count() - 1;

    position_storage[index] = glm::vec4(temp, 0.0f);
    hex_tile_storage[index] = HexTileUBO();

    tile_setup(temp, &hex_tile_storage[index]);

    glNamedBufferStorage(tile_position_buffer, sizeof(glm::vec3) * gm_controller->tiles_count, position_storage, GL_DYNAMIC_STORAGE_BIT);

    glNamedBufferStorage(tile_data, sizeof(HexTileUBO) * gm_controller->tiles_count, hex_tile_storage, GL_DYNAMIC_STORAGE_BIT);

    tile_setup(active_tile_storage);

    glNamedBufferStorage(active_tile_data, sizeof(ActiveHexTileUBO), active_tile_storage, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(selected_object_buffer, sizeof(ObjectUBO), selected_object_ubo, GL_DYNAMIC_STORAGE_BIT);

    glm::vec4 tempVec4 = glm::vec4(0.0f, visualize_height, 0.0f, 0.0f);
    glNamedBufferStorage(visualize_placement, sizeof(ActiveHexTileUBO), active_tile_storage, GL_DYNAMIC_STORAGE_BIT);

    QOL::SubBufferType args1 = QOL::SubBufferType((size_t)visualize_placement, (size_t)0, (size_t)sizeof(glm::vec4));
    QOL::SubBufferType args2 = QOL::SubBufferType((size_t)selected_object_buffer, (size_t)0, (size_t)sizeof(glm::mat4));

    visualize_movement_transitions = new Timer<glm::vec4, QOL::SubBufferType>(300, animation_functions::ease_in_ease_out, QOL::basicTimer<glm::vec4>, tempVec4, glm::vec4(1.0f));
    // visualize_movement_transitions->directChange(args1, QOL::ChangeBufferSubData);

    visualize_rotation_transitions = new Timer<float, QOL::SubBufferType>(300, animation_functions::ease_in_ease_out, QOL::hexagonRotation, 0.0f, 1.0f);
    // visualize_rotation_transitions->directChange(args2, QOL::ChangeMatrixRotationSubData);


    ORS::ORS_instanced hexagonRS = ORS::ORS_instanced(
                                        new ORS::ArrayData(GL_TRIANGLES, 0, 54),
                                        &main_program
                                        );

    ORS::BufferData* hexagonBuffers = new ORS::BufferData[7]{
            ORS::BufferData(GL_UNIFORM_BUFFER, 0, &camera_buffer),
            ORS::BufferData(GL_UNIFORM_BUFFER, 1, &light_buffer),
            ORS::BufferData(GL_UNIFORM_BUFFER, 2, &objects_buffer),

            ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 3, &tile_data),
            ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 4, &tile_position_buffer),
            ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 5, &color_buffer),
            ORS::BufferData(GL_VERTEX_ARRAY, 0, &hexagon_pos_indexed_vao)
        };

    hexagonRS.SetBuffers(hexagonBuffers, 7);

    objectInstancedStorage.push_back(hexagonRS);

    ORS::ORS selectedRS = ORS::ORS(
                                    new ORS::ArrayData(GL_TRIANGLES, 0 , 54),
                                    &selected_tile_program
                                );

    ORS::BufferData* selectedBuffers = new ORS::BufferData[5]{
            ORS::BufferData(GL_UNIFORM_BUFFER, 0, &camera_buffer),
            ORS::BufferData(GL_UNIFORM_BUFFER, 1, &light_buffer),
            ORS::BufferData(GL_UNIFORM_BUFFER, 2, &selected_object_buffer),

            ORS::BufferData(GL_UNIFORM_BUFFER, 3, &visualize_placement),
            ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 4, &color_buffer),
        };
    
    selectedRS.SetBuffers(selectedBuffers, 5);

    objectStorage.push_back(selectedRS);

    // details:

    details = new std::map<int, std::vector<ORS::ORS_instanced>*>();

    const auto [i, success] = details->insert({(int)CITY, new std::vector<ORS::ORS_instanced>()});

    std::filesystem::path adresses[2] = {"building-sample-house-b", "building-sample-tower-a"};
    int adresses_areas[2] = {CITY, CITY};

    CreateObjectsORS(adresses, adresses_areas, 2);

    compile_shaders();
}

Application::~Application() {

    delete_shaders();

    glDeleteBuffers(1, &camera_buffer);
    glDeleteBuffers(1, &selected_camera_buffer);
    glDeleteBuffers(1, &light_buffer);
    glDeleteBuffers(1, &objects_buffer);
    glDeleteBuffers(1, &selected_object_buffer);
    glDeleteBuffers(objects_allocated, object_buffers);

    glDeleteBuffers(1, &tile_data);
    glDeleteBuffers(1, &active_tile_data);
    glDeleteBuffers(1, &visualize_placement);
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
    selected_tile_program = create_program(lecture_shaders_path / "selected.vert", lecture_shaders_path / "selected.frag");
    textured_objects_program = create_program(lecture_shaders_path / "textured.vert", lecture_shaders_path / "textured.frag");
}

void Application::update(float delta) {}

void Application::render() {
    //animations

    glm::vec4 selected_pos = visualize_movement_transitions->next_value();
    float selected_rotate_by = visualize_rotation_transitions->next_value();

    glm::mat4 selected_tile_mat = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(selected_pos)), selected_rotate_by * 3.14f / 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 active_tile_mat = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)), selected_rotate_by * 3.14f / 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    glNamedBufferSubData((size_t)selected_object_buffer, (size_t)0, (size_t)sizeof(glm::mat4), &selected_tile_mat);

    // rest
    camera_ubo.position = glm::vec4(camera.get_eye_position(), 1.0f);
    camera_ubo.projection = glm::perspective(glm::radians(45.0f), static_cast<float>(width * 0.75f) / static_cast<float>(height), 0.01f, 1000.0f);
    camera_ubo.view = glm::lookAt(camera.get_eye_position(), camera.look_at, glm::vec3(0.0f, 1.0f, 0.0f));
    glNamedBufferSubData(camera_buffer, 0, sizeof(CameraUBO), &camera_ubo);

    selected_camera_ubo.position = glm::vec4(selected_camera.get_eye_position(), 1.0f);
    selected_camera_ubo.projection = glm::perspective(glm::radians(45.0f), (float(width) * 0.25f) / (float(height) * 0.25f), 0.01f, 100.0f);
    selected_camera_ubo.view = glm::lookAt(selected_camera.get_eye_position(), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glNamedBufferSubData(selected_camera_buffer, 0, sizeof(CameraUBO), &selected_camera_ubo);

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
    (*(*details)[CITY])[0].render();

    objectInstancedStorage[0].object_count = gm_controller->tile_count();
    objectInstancedStorage[0].render();

    objectStorage[0].render();

    glNamedBufferSubData((size_t)selected_object_buffer, (size_t)0, (size_t)sizeof(glm::mat4), &active_tile_mat);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, selected_camera_buffer);
    glViewport((GLsizei)(width * 0.75f), (GLsizei)(height * 0.75f), (GLsizei)(width * 0.25f), (GLsizei)(height * 0.25f));

    glDrawArrays(GL_TRIANGLES, 0, 54);

    glm::vec3 mousePos = RayCast();

    glm::vec3 tilePosition = glm::vec3(QOL::roundClossest(mousePos.x, SQRTDIST), 0, QOL::roundClossest(mousePos.z, 0.5f));

    if(gm_controller->optional_map.contains(gm_controller->SpaceToVec(tilePosition))){
        visualize_movement_transitions->changeCurEnd(glm::vec4(tilePosition.x, -visualize_height, tilePosition.z, 0.0f), true);
    }
}

void Application::render_ui() { 
    const float unit = ImGui::GetFontSize();
    ImGui::Begin("Another Window", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::SetWindowSize(ImVec2(0.21f * width, 0.71f * height));
    ImGui::SetWindowPos(ImVec2(0.77f * width, 0.27f * height));
    ImGui::Text("current score: %d", gm_controller->get_score());
    ImGui::End();
}

// ----------------------------------------------------------------------------
// Input Events
// ----------------------------------------------------------------------------

void Application::on_resize(int width, int height) {
    // Calls the default implementation to set the class variables.
    PV112Application::on_resize(width, height);
}

void Application::on_mouse_move(double x, double y) { 
    camera.on_mouse_move(x, y);
    selected_camera.on_mouse_move(x, y);
}
void Application::on_mouse_button(int button, int action, int mods) {
    if (action == GLFW_RELEASE) {
        AddTile();
    }
    camera.on_mouse_button(button, action, mods);
    selected_camera.on_mouse_button(button, action, mods);
}

glm::vec3 Application::RayCast(){

    glm::vec3 aux = glm::vec3(0.0f);
    double RCx = 0.0;
    double RCy = 0.0;
    glfwGetCursorPos(window, &RCx, &RCy);

    float mouseX = (float)RCx / ((width * 0.75f) * 0.5f) - 1.0f;
    float mouseY = (float)RCy / (height * 0.5f) - 1.0f;

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), static_cast<float>(width * 0.75f) / static_cast<float>(height), 0.01f, 1000.0f);
    glm::mat4 view = glm::lookAt(camera.get_eye_position(), camera.look_at, glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 invVP = glm::inverse(proj * view);
    glm::vec4 screenPos = glm::vec4(-mouseX, mouseY, 1.0f, 1.0f);
    glm::vec4 worldPos = invVP * screenPos;

    glm::vec3 dir = glm::normalize(glm::vec3(worldPos));

    float temp = camera.get_distance();

    aux = dir * temp - camera.look_at * 2 + camera.get_eye_position();
    aux.y = 0;
    return -aux;
}

void Application::on_key_pressed(int key, int scancode, int action, int mods) {
    // Calls default implementation that invokes compile_shaders when 'R key is hit.
    #ifdef DEBUG
    PV112Application::on_key_pressed(key, scancode, action, mods);
    #endif
    camera.move(key, action);
    if(action != GLFW_RELEASE) return;
    switch(key){
        int rotate_to;
        case GLFW_KEY_E:
            rotate_to = (--gm_controller->selected_tile->rotation + 6) % 6;
            gm_controller->selected_tile->rotation = rotate_to;
            visualize_rotation_transitions->changeCurEnd((float)rotate_to, true);
            break;
        case GLFW_KEY_Q:
            rotate_to = (++gm_controller->selected_tile->rotation) % 6;
            gm_controller->selected_tile->rotation = rotate_to;
            visualize_rotation_transitions->changeCurEnd((float)rotate_to, true);
            break;
    }

}
// MY FUNCTIONS

void Application::AddTile(){
    glm::vec3 mousePos = RayCast();

    glm::vec3 tilePosition = glm::vec3(QOL::roundClossest(mousePos.x, SQRTDIST), 0, QOL::roundClossest(mousePos.z, 0.5f));
    glm::vec3 clossest = glm::vec3(tilePosition);
    float dist = QOL::pythDist(mousePos, tilePosition);
    for (size_t i = 0; i < 6; i++)
    {
        glm::vec3 x = tilePosition + (glm::vec3)OFFSETMAP[i];
        float xDist = QOL::pythDist(x, mousePos);
        if(xDist < dist) {
            clossest = x;
            dist = xDist;
        }
        if (dist < SQRTDIST / 2) break;
    }

    glm::ivec3 vectorizedPos = gm_controller->SpaceToVec(clossest);

    if(gm_controller->play(clossest)){
        int index = gm_controller->tile_count() - 1;

        HexTileUBO tileUBO = HexTileUBO();

        tile_setup(vectorizedPos, &tileUBO);
        glm::vec4 tilePosVec4 = glm::vec4(clossest, 0.0f);
        glNamedBufferSubData(tile_position_buffer, index * sizeof(glm::vec4), sizeof(glm::vec4), &tilePosVec4);
        glNamedBufferSubData(tile_data, index * sizeof(HexTileUBO), sizeof(HexTileUBO), &tileUBO);

        ActiveHexTileUBO selectedTileUBO = ActiveHexTileUBO();
        tile_setup(&selectedTileUBO);

        glNamedBufferSubData(active_tile_data, 0, sizeof(HexTileUBO), &selectedTileUBO);
        glNamedBufferSubData(visualize_placement, 0, sizeof(HexTileUBO), &selectedTileUBO);
    }
}

void Application::tile_setup(glm::ivec3 pos, HexTileUBO* adress){
    GameTileData* first_tile = gm_controller->selected_tile;

    first_tile = nullptr;
    if(!gm_controller->game_map.get_data(pos, &first_tile)){
        assert(false);
    }
    

    adress->position = glm::vec4(pos, 1);

    adress->rotation = first_tile->rotation;
    adress->target = first_tile->target;

    adress->triangle1 = first_tile->hex_triangles[0];
    adress->triangle2 = first_tile->hex_triangles[1];
    adress->triangle3 = first_tile->hex_triangles[2];
    adress->triangle4 = first_tile->hex_triangles[3];
    adress->triangle5 = first_tile->hex_triangles[4];
    adress->triangle6 = first_tile->hex_triangles[5];

    adress->river1 = glm::vec4(0);
    adress->river2 = glm::vec4(0);
    adress->rail1 = glm::vec4(0);
    adress->rail2 = glm::vec4(0);
}

void Application::tile_setup(ActiveHexTileUBO* adress){
    GameTileData* first_tile = gm_controller->selected_tile;

    adress->position = glm::vec4(0, 0, 0, 1);

    adress->rotation = (float)(first_tile->rotation);
    adress->target = first_tile->target;

    adress->triangle1 = first_tile->hex_triangles[0];
    adress->triangle2 = first_tile->hex_triangles[1];
    adress->triangle3 = first_tile->hex_triangles[2];
    adress->triangle4 = first_tile->hex_triangles[3];
    adress->triangle5 = first_tile->hex_triangles[4];
    adress->triangle6 = first_tile->hex_triangles[5];

    adress->river1 = glm::vec4(0);
    adress->river2 = glm::vec4(0);
    adress->rail1 = glm::vec4(0);
    adress->rail2 = glm::vec4(0);
}

void Application::add_details(HexTileUBO* adress){


    Timer<glm::mat4, QOL::SubBufferType>* temp = new Timer<glm::mat4, QOL::SubBufferType>(150, animation_functions::ease_in_ease_out, QOL::basicTimer, glm::mat4(0.0f), glm::mat4(1.0f), 40);
    
    // TODO fix subbuffer values
    temp->directChange(QOL::SubBufferType(1, 2, 3), QOL::ChangeMatrixSubData);
    timers->push_back((Timer<void*, QOL::SubBufferType>*) temp);
}

void Application::CreateObjectsORS(std::filesystem::path* files, int* areas, GLsizei size){
    object_buffers = new GLuint[size];
    glCreateBuffers(size, object_buffers);
    objects_allocated = size;
    for (size_t i = 0; i < size; i++)
    {
        (*details)[areas[i]]->push_back(ORSSetup(files[i], i));
    }
}

ORS::ORS_instanced Application::ORSSetup(std::filesystem::path name, size_t index){
    std::filesystem::path object_path = objects_path / name;
    object_path.replace_extension(".obj");
    Geometry object_geometry = Geometry::from_file(object_path);
    object_path = images_path / name;
    object_path.replace_extension(".jpg");
    ORS::TextureData object_texture = ORS::TextureData(3, load_texture_2d(object_path));
    ORS::ORS_instanced object_instancer = ORS::ORS_instanced(&object_geometry, &object_texture, &textured_objects_program);

    ORS::BufferData* object_buffer_data = new ORS::BufferData[2]{
            ORS::BufferData(GL_UNIFORM_BUFFER, 0, &camera_buffer),
            ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 1, &light_buffer),
        };

    ObjectUBO object_UBO = ObjectUBO {.model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.15, 0.0)), glm::vec3(0.1f)),
                                                .ambient_color = glm::vec4(0.0f),
                                                .diffuse_color = glm::vec4(1.0f),
                                                .specular_color = glm::vec4(0.5f)};

    object_instancer.SetBuffers(object_buffer_data, 2);

    if(!glIsBuffer(object_buffers[index])){
        assert(false);
    }

    glNamedBufferStorage(object_buffers[index], sizeof(ObjectUBO) * 10, &object_UBO, GL_DYNAMIC_STORAGE_BIT);

    ORS::BufferData object_dynamic_buffer_data = ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 2, &(object_buffers[index]));
    object_instancer.object_count = 1;
    object_instancer.AddDynamicBuffer(&object_dynamic_buffer_data, 10);

    std::cout << object_texture.adress;
    std::cout << ", ";
    std::cout << object_texture.texture;
    std::cout << "\n";

    return object_instancer;
}


namespace QOL{
    void ChangeMatrixSubData(FunctionType<glm::mat4, SubBufferType> data){
        glNamedBufferSubData(data.args.buffer, data.args.index, data.args.size, &(data.data));
    }

    void ChangeMatrixRotationSubData(FunctionType<float, SubBufferType> data){
        glm::mat4 newMat = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0)), data.data / 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        glNamedBufferSubData(data.args.buffer, data.args.index, data.args.size, &newMat);
    }
}
