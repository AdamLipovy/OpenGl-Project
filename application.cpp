#include "application.hpp"

#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using std::make_shared;

#define visualize_height -0.1f

double temp1 (float x){return (double)x;}

float temp2 (double x, float a, float b) {return a;}

GLuint load_texture_2d(const std::filesystem::path filename, int* widthptr = nullptr, int *heightptr = nullptr) {
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

    if(widthptr != nullptr)
        *widthptr = width;

    if(heightptr != nullptr)
        *heightptr = width;

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

    qTexture = ImageData(0, 0, 0);
    qTexture.Id = load_texture_2d(images_path / "keyboard_q_outline.png", &(qTexture.image_width), &(qTexture.image_height));
    wTexture = ImageData(0, 0, 0);
    wTexture.Id = load_texture_2d(images_path / "keyboard_w_outline.png", &(wTexture.image_width), &(wTexture.image_height));
    eTexture = ImageData(0, 0, 0);
    eTexture.Id = load_texture_2d(images_path / "keyboard_e_outline.png", &(eTexture.image_width), &(eTexture.image_height));
    aTexture = ImageData(0, 0, 0);
    aTexture.Id = load_texture_2d(images_path / "keyboard_a_outline.png", &(aTexture.image_width), &(aTexture.image_height));
    sTexture = ImageData(0, 0, 0);
    sTexture.Id = load_texture_2d(images_path / "keyboard_s_outline.png", &(sTexture.image_width), &(sTexture.image_height));
    dTexture = ImageData(0, 0, 0);
    dTexture.Id = load_texture_2d(images_path / "keyboard_d_outline.png", &(dTexture.image_width), &(dTexture.image_height));

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
    glVertexArrayAttribFormat(hexagon_pos_indexed_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(hexagon_pos_indexed_vao, 0, 0);

    glVertexArrayVertexBuffer(hexagon_pos_indexed_vao, 1, hexagon_normal_vertexes, 0, 3 * sizeof(float));

    glEnableVertexArrayAttrib(hexagon_pos_indexed_vao, 1);
    glVertexArrayAttribFormat(hexagon_pos_indexed_vao, 1, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(hexagon_pos_indexed_vao, 1, 1);

    // --------------------------------------------------------------------------
    // Initialize UBO Data
    // --------------------------------------------------------------------------
    camera_ubo.position = glm::vec4(camera.get_eye_position(), 1.0f);
    camera_ubo.projection = glm::perspective(glm::radians(45.0f), (float(width) * 0.75f) / float(height), 0.01f, 100.0f);
    camera_ubo.view = glm::lookAt(camera.get_eye_position(), camera.look_at, glm::vec3(0.0f, 1.0f, 0.0f));

    lights.push_back({.position = glm::vec4(0.0, 50, 0.75, 1.0),
                                 .ambient_color = glm::vec4(0.5f),
                                 .diffuse_color = glm::vec4(1.0f),
                                 .specular_color = glm::vec4(1.0f)});

    objects_ubos.push_back({.model_matrix = glm::mat4(1.0f),
                            .ambient_color = glm::vec4(1.0f),
                            .diffuse_color = glm::vec4(0.5f),
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

    glGenBuffers(1, &light_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LightUBO) * 1000, lights.data(), GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, light_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

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

    glGenBuffers(1, &tile_data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, tile_data);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(HexTileUBO) * gm_controller->tiles_count, hex_tile_storage, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, tile_data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    tile_setup(active_tile_storage);

    glNamedBufferStorage(active_tile_data, sizeof(ActiveHexTileUBO), active_tile_storage, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(selected_object_buffer, sizeof(ObjectUBO), selected_object_ubo, GL_DYNAMIC_STORAGE_BIT);

    glm::vec4 tempVec4 = glm::vec4(0.0f, visualize_height, 0.0f, 0.0f);
    glNamedBufferStorage(visualize_placement, sizeof(ActiveHexTileUBO), active_tile_storage, GL_DYNAMIC_STORAGE_BIT);

    QOL::SubBufferType args1 = QOL::SubBufferType((size_t)visualize_placement, (size_t)0, (size_t)sizeof(glm::vec4));
    QOL::SubBufferType args2 = QOL::SubBufferType((size_t)selected_object_buffer, (size_t)0, (size_t)sizeof(glm::mat4));

    visualize_movement_transitions = new Timer<glm::vec4, QOL::SubBufferType>(300, animation_functions::ease_in_ease_out, QOL::basicTimer<glm::vec4>, tempVec4, glm::vec4(0.0f));

    visualize_rotation_transitions = new Timer<float, QOL::SubBufferType>(300, animation_functions::ease_in_ease_out, QOL::hexagonRotation, 0.0f, 0.0f);


    ORS::ORS_instanced* hexagonRS = new ORS::ORS_instanced(
                                        new ORS::ArrayData(GL_TRIANGLES, 0, 54),
                                        &main_program
                                        );

    ORS::BufferData* hexagonBuffers = new ORS::BufferData[8]{
            ORS::BufferData(GL_UNIFORM_BUFFER, 0, &camera_buffer),
            ORS::BufferData(GL_UNIFORM_BUFFER, 1, &light_buffer),
            ORS::BufferData(GL_UNIFORM_BUFFER, 2, &objects_buffer),

            ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 3, &tile_data),
            ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 4, &tile_position_buffer),
            ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 5, &color_buffer),
            ORS::BufferData(GL_VERTEX_ARRAY, 0, &hexagon_pos_indexed_vao),
            ORS::BufferData(GL_MAP1_INDEX, 6, &lights_count)
        };

    hexagonRS->SetBuffers(hexagonBuffers, 8);

    objectInstancedStorage.push_back(hexagonRS);

    ORS::ORS* selectedRS = new ORS::ORS(
                                    new ORS::ArrayData(GL_TRIANGLES, 0 , 54),
                                    &selected_tile_program
                                );

    ORS::BufferData* selectedBuffers = new ORS::BufferData[7]{
            ORS::BufferData(GL_UNIFORM_BUFFER, 0, &camera_buffer),
            ORS::BufferData(GL_UNIFORM_BUFFER, 1, &light_buffer),
            ORS::BufferData(GL_UNIFORM_BUFFER, 2, &selected_object_buffer),
            ORS::BufferData(GL_MAP1_INDEX, 5, &lights_count),         // COLOR COUNT LINK

            ORS::BufferData(GL_UNIFORM_BUFFER, 3, &visualize_placement),
            ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 4, &color_buffer),
            ORS::BufferData(GL_VERTEX_ARRAY, 0, &hexagon_pos_indexed_vao),
        };
    
    selectedRS->SetBuffers(selectedBuffers, 7);

    objectStorage.push_back(selectedRS);

    // details:

    details->insert({(int)NONE, new std::vector<ORS::ORS_instanced*>()});
    details->insert({(int)MEADOW, new std::vector<ORS::ORS_instanced*>()});
    details->insert({(int)FIELD, new std::vector<ORS::ORS_instanced*>()});
    details->insert({(int)FOREST, new std::vector<ORS::ORS_instanced*>()});
    details->insert({(int)CITY, new std::vector<ORS::ORS_instanced*>()});
    details->insert({(int)RAIL, new std::vector<ORS::ORS_instanced*>()});
    details->insert({(int)WATER, new std::vector<ORS::ORS_instanced*>()});

    CreateObjectsORS(adresses, adresses_areas, object_count);

    add_details(hex_tile_storage[index], temp);

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
    glm::vec4 selected_pos = visualize_movement_transitions->next_value();
    float selected_rotate_by = visualize_rotation_transitions->next_value();

    std::for_each(timers->begin(), timers->end(), [](Timer<QOL::MatChange, QOL::SubBufferType>* clock) {clock->next_value();} );
    bool resize = false;
    for(int i = 0; i < timers->size(); i++){
        if(!(*timers)[i]->is_active()){
            timers->erase(timers->begin() + i);
            resize = true;
        }
    }

    if(resize) timers->shrink_to_fit();

    glm::mat4 selected_tile_mat = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(selected_pos)), selected_rotate_by * 3.14f / 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 active_tile_mat = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)), selected_rotate_by * 3.14f / 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    glNamedBufferSubData((size_t)selected_object_buffer, (size_t)0, (size_t)sizeof(glm::mat4), &selected_tile_mat);

    lights_count = lights.size();

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
    for(int i = 0; i < 7; i++){
        for(int o = 0; o < (*(*details)[i]).size(); o++){
            (*(*details)[i])[o]->render();
        }
    }

    objectInstancedStorage[0]->object_count = gm_controller->tile_count();
    objectInstancedStorage[0]->render();

    objectStorage[0]->render();

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
    if(ImGui::ImageButton((ImTextureID)(intptr_t)(qTexture.Id), ImVec2(qTexture.image_width, qTexture.image_height),
                                ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0.5, 0.5, 0.5, 0), ImVec4(0.5, 0.5, 0.5, 1)))
                                { on_key_pressed(GLFW_KEY_Q, 0, GLFW_RELEASE, 0); }
    ImGui::SameLine();
    if(ImGui::ImageButton((ImTextureID)(intptr_t)(wTexture.Id), ImVec2(wTexture.image_width, wTexture.image_height),
                                ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0.5, 0.5, 0.5, 0), ImVec4(0.5, 0.5, 0.5, 1)))
                                { on_key_pressed(GLFW_KEY_W, 0, GLFW_RELEASE, 0); }
    ImGui::SameLine();
    if(ImGui::ImageButton((ImTextureID)(intptr_t)(eTexture.Id), ImVec2(eTexture.image_width, eTexture.image_height),
                                ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0.5, 0.5, 0.5, 0), ImVec4(0.5, 0.5, 0.5, 1)))
                                { on_key_pressed(GLFW_KEY_E, 0, GLFW_RELEASE, 0); }
    if(ImGui::ImageButton((ImTextureID)(intptr_t)(aTexture.Id), ImVec2(aTexture.image_width, aTexture.image_height),
                                ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0.5, 0.5, 0.5, 0), ImVec4(0.5, 0.5, 0.5, 1)))
                                { on_key_pressed(GLFW_KEY_A, 0, GLFW_RELEASE, 0); }
    ImGui::SameLine();
    if(ImGui::ImageButton((ImTextureID)(intptr_t)(sTexture.Id), ImVec2(sTexture.image_width, sTexture.image_height),
                                ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0.5, 0.5, 0.5, 0), ImVec4(0.5, 0.5, 0.5, 1)))
                                { on_key_pressed(GLFW_KEY_S, 0, GLFW_RELEASE, 0); }
    ImGui::SameLine();
    if(ImGui::ImageButton((ImTextureID)(intptr_t)(dTexture.Id), ImVec2(dTexture.image_width, dTexture.image_height),
                                ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0.5, 0.5, 0.5, 0), ImVec4(0.5, 0.5, 0.5, 1)))
                                { on_key_pressed(GLFW_KEY_D, 0, GLFW_RELEASE, 0); }
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
    if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
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
    PV112Application::on_key_pressed(key, scancode, action, mods);
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
        add_details(tileUBO, clossest);
        glm::vec4 tilePosVec4 = glm::vec4(clossest, 0.0f);
        glNamedBufferSubData(tile_position_buffer, index * sizeof(glm::vec4), sizeof(glm::vec4), &tilePosVec4);
        glNamedBufferSubData(tile_data, index * sizeof(HexTileUBO), sizeof(HexTileUBO), &tileUBO);

        ActiveHexTileUBO selectedTileUBO = ActiveHexTileUBO();
        tile_setup(&selectedTileUBO);

        glNamedBufferSubData(active_tile_data, 0, sizeof(HexTileUBO), &selectedTileUBO);
        glNamedBufferSubData(visualize_placement, 0, sizeof(HexTileUBO), &selectedTileUBO);

        visualize_rotation_transitions->changeCurEnd(0, true);
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

void Application::initialize_detail(glm::vec3 tile_middle_pos, int delay, QOL::RenderObject* diffData, int type, int index, QOL::MatChange data){
    glm::vec3 offset_pos = glm::vec3((rand() % 100) / 750.0f, 0.0f, (rand() % 100) / 750.0f);
    glm::vec3 position = offset_pos + tile_middle_pos;
    Timer<QOL::MatChange, QOL::SubBufferType>* timer = new Timer<QOL::MatChange, QOL::SubBufferType>(150,
                                                        animation_functions::ease_in_ease_out, QOL::matTransition,
                                                        QOL::MatChange(data.rotate - 6, data.rotate_around, position, glm::vec3(0.001f)),
                                                        QOL::MatChange(data.rotate, data.rotate_around, position + data.position, data.size),
                                                        delay);
    if((*(*details)[type]).size() <= index) { return; }
    GLsizei count = (*(*details)[type])[index]->object_count;
    GLuint* buffer = (*(*details)[type])[index]->GetDynamicBufferAdress();
    (*(*details)[type])[index]->AddInstance();

    glNamedBufferSubData(*buffer, count * sizeof(QOL::RenderObject), sizeof(QOL::RenderObject), diffData);
    timer->directChange(QOL::SubBufferType(*buffer, count * sizeof(QOL::RenderObject), sizeof(glm::mat4)), QOL::CreateMatrix);
    timers->push_back(timer);
}

void Application::add_details(HexTileUBO adress, glm::vec3 position){

    int hex_data[6] = {adress.triangle1, adress.triangle2, adress.triangle3, adress.triangle4, adress.triangle5, adress.triangle6};
    int delay = 100;
    glm::vec3 fixed_pos = position;
    if(position != glm::vec3(0.0f)){
        // fixed_pos += glm::vec3(-0.05f, 0.0f, -0.2f);
    }
    QOL::RenderObject* diffData = new QOL::RenderObject(glm::mat4(1.0f), glm::vec4(0.0f), glm::vec4(1.0f), glm::vec4(0.0f));
    for (size_t i = 0; i < 6; i++)
    {
        int triangle = (i - adress.rotation + 6) % 6;
        switch (hex_data[i])
        {
        case CITY:
            for (size_t x = 0; x < 3; x++)
            {
                initialize_detail(TRIANGLE_MIDDLES[triangle] + fixed_pos, delay, diffData, CITY, 0,
                                    QOL::MatChange{(float)(rand() % 6), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.1f, 0.0f), glm::vec3(0.1f)});
                delay += 30;
            }
            initialize_detail(TRIANGLE_MIDDLES[triangle] + fixed_pos, delay, diffData, CITY, 1,
                                QOL::MatChange{(float)(rand() % 6), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.1f, 0.0f), glm::vec3(0.1f)});
            delay += 30;

            initialize_detail(TRIANGLE_MIDDLES[triangle] + fixed_pos, delay, diffData, CITY, 2,
                                QOL::MatChange{(float)(rand() % 6), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.05f, 0.0f), glm::vec3(0.05f)});

            glm::vec3 campfire_position = TRIANGLE_MIDDLES[i] + fixed_pos;
            campfire_position.y = 0.2f;

            LightUBO newLight = LightUBO{.position = glm::vec4(campfire_position, 1.0f),
                                        .ambient_color = glm::vec4(0.0f),
                                        .diffuse_color = glm::vec4(1.1f, 1.1f, 1.0f, 1.0f),
                                        .specular_color = glm::vec4(0.0f)};

            glNamedBufferSubData(light_buffer, lights_count * sizeof(LightUBO), sizeof(LightUBO), &newLight);
            lights_count++;
            lights.push_back(newLight);

            delay += 30;
            break;

        case FOREST:
            for (size_t x = 0; x < (rand() % 6); x++)
            {
                float size = (rand() % 10) / 100.0f;
                initialize_detail(TRIANGLE_MIDDLES[triangle] + fixed_pos, delay, diffData, FOREST, 0,    
                                QOL::MatChange{(float)(rand() % 6), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, size, 0.0f), glm::vec3(0.2f + size)});
                delay += 30;
            }
            for (size_t x = 0; x < (rand() % 6); x++)
            {
                float size = (rand() % 10) / 100.0f;
                initialize_detail(TRIANGLE_MIDDLES[triangle] + fixed_pos, delay, diffData, FOREST, 1,    
                                QOL::MatChange{(float)(rand() % 6), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, size, 0.0f), glm::vec3(0.2f + size)});
                delay += 30;
            }

            delay += 30;
            break;
        case FIELD:
            for (size_t x = 0; x < (rand() % 6); x++)
            {
                float size = (rand() % 10) / 500.0f;
                initialize_detail(TRIANGLE_MIDDLES[triangle] + fixed_pos, delay, diffData, FIELD, 0,    
                                QOL::MatChange{(float)(rand() % 6), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, size, 0.0f), glm::vec3(0.1f + size)});
                delay += 30;
            }

            delay += 30;
            break;
        }
    }
}

void Application::CreateObjectsORS(const std::filesystem::path* files, const int* areas, GLsizei size){
    object_buffers = new GLuint[size];
    glGenBuffers(size, object_buffers);
    objects_allocated = size;
    std::filesystem::path targets[4] = {"3target.png", "4target.png", "5target.png", "6target.png"};
    for (size_t i = 0; i < size; i++)
    {
        if(areas[i] != NONE){
            (*details)[areas[i]]->push_back(ORSSetup(files[i], i));
        } else{
            // TODO 
        //     std::filesystem::path object_path = objects_path / files[i];
        //     object_path.replace_extension(".obj");
        //     for(int i = 0; i < 4; i++){

        //         (*details)[areas[i]]->push_back(ORSSetup(object_path, targets[i], &(object_buffers[i])));
        //     }
        }
    }
}

ORS::ORS_instanced* Application::ORSSetup(std::filesystem::path name, size_t index){
    std::filesystem::path object_path = objects_path / name;
    object_path.replace_extension(".obj");
    Geometry* object_geometry = new Geometry();
    *object_geometry = Geometry::from_file(object_path);
    object_path = images_path / name;
    object_path.replace_extension(".jpg");
    ORS::TextureData* object_texture = new ORS::TextureData(5, load_texture_2d(object_path));
    ORS::ORS_instanced* object_instancer = new ORS::ORS_instanced(object_geometry, object_texture, &textured_objects_program);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, object_buffers[index]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ObjectUBO) * 1000, nullptr, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, object_buffers[index]);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    ORS::BufferData* object_buffer_data = new ORS::BufferData[4]{
            ORS::BufferData(GL_UNIFORM_BUFFER, 0, &camera_buffer),
            ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 1, &light_buffer),
            ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 2, &(object_buffers[index])),
            ORS::BufferData(GL_MAP1_INDEX, 4, &lights_count),
        };

    object_instancer->SetBuffers(object_buffer_data, 4);


    ORS::BufferData object_dynamic_buffer_data = ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 2, &(object_buffers[index]));
    object_instancer->object_count = 0;
    object_instancer->AddDynamicBuffer(&object_dynamic_buffer_data, 10);

    return object_instancer;
}

ORS::ORS_instanced* Application::ORSSetup(std::filesystem::path object, std::filesystem::path texture, GLuint* buffer){
    Geometry object_geometry = Geometry::from_file(object);
    ORS::TextureData object_texture = ORS::TextureData(3, load_texture_2d(texture));
    ORS::ORS_instanced* object_instancer = new ORS::ORS_instanced(&object_geometry, &object_texture, &textured_objects_program);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, *buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ObjectUBO) * 1000, nullptr, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, *buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    ORS::BufferData* object_buffer_data = new ORS::BufferData[4]{
            ORS::BufferData(GL_UNIFORM_BUFFER, 0, &camera_buffer),
            ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 1, &light_buffer),
            ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 2, buffer),
            ORS::BufferData(GL_MAP1_INDEX, 4, &lights_count),
        };

    ObjectUBO object_UBO = ObjectUBO {.model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0)), glm::vec3(0.3f)),
                                                .ambient_color = glm::vec4(0.0f),
                                                .diffuse_color = glm::vec4(1.0f),
                                                .specular_color = glm::vec4(0.5f)};

    object_instancer->SetBuffers(object_buffer_data, 4);

    ORS::BufferData object_dynamic_buffer_data = ORS::BufferData(GL_SHADER_STORAGE_BUFFER, 2, buffer);
    object_instancer->object_count = 0;
    object_instancer->AddDynamicBuffer(&object_dynamic_buffer_data, 10);

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

    void CreateMatrix(FunctionType<MatChange, SubBufferType> data){
        glm::mat4 newMat = glm::translate(glm::mat4(1.0f), data.data.position);
        newMat = glm::rotate(newMat, data.data.rotate / 3.0f, data.data.rotate_around);
        newMat = glm::scale(newMat, data.data.size);
        glNamedBufferSubData(data.args.buffer, data.args.index, data.args.size, &newMat);
    }
}
