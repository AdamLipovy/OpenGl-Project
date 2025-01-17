
#include "SceneCameras.hpp"

#define GLFW_KEY_A                  65
#define GLFW_KEY_D                  68
#define GLFW_KEY_S                  83
#define GLFW_KEY_W                  87

#define CHANGEBY 0.2f

const float MovingCamera::max_distance = 10.0f;

void MovingCamera::update_eye_pos(){
    if (switch_axes) {
        eye_position.x = look_at.x + distance * cosf(angle_elevation) * sinf(angle_direction);
        eye_position.z = look_at.z + distance * sinf(angle_elevation);
        eye_position.y = look_at.y + distance * cosf(angle_elevation) * cosf(angle_direction);
    } else {
        eye_position.x = look_at.x + distance * cosf(angle_elevation) * -sinf(angle_direction);
        eye_position.y = look_at.y + distance * sinf(angle_elevation);
        eye_position.z = look_at.z + distance * cosf(angle_elevation) * cosf(angle_direction);
    }
}

void MovingCamera::on_mouse_move(double x, double y){
    float dx = static_cast<float>(x - last_x);
    float dy = static_cast<float>(y - last_y);
    last_x = static_cast<int>(x);
    last_y = static_cast<int>(y);

    if (is_rotating) {
        angle_direction += dx * angle_sensitivity;
        angle_elevation += dy * angle_sensitivity;

        // Clamps the results.
        angle_elevation = glm::clamp(angle_elevation, 0.2f, 1.5f);
    }
    if (is_zooming) {
        distance *= (1.0f + dy * zoom_sensitivity);

        // Clamps the results.
        if (distance < min_distance)
            distance = min_distance;
        if (distance > max_distance)
            distance = max_distance;
    }
    if (is_rotating || is_zooming){ update_eye_pos(); }
}

void MovingCamera::move(int key, int action) {
    switch (key)
    {
    case GLFW_KEY_A:
        eye_position.x += CHANGEBY * -cosf(angle_direction);
        eye_position.z += CHANGEBY * -sinf(angle_direction);
        look_at.x += CHANGEBY * -cosf(angle_direction);
        look_at.z += CHANGEBY * -sinf(angle_direction);
        break;
    case GLFW_KEY_D:
        eye_position.x += CHANGEBY * cosf(angle_direction);
        eye_position.z += CHANGEBY * sinf(angle_direction);
        look_at.x += CHANGEBY * cosf(angle_direction);
        look_at.z += CHANGEBY * sinf(angle_direction);
        break;
    case GLFW_KEY_S:
        eye_position.x += CHANGEBY * -sinf(angle_direction);
        eye_position.z += CHANGEBY * cosf(angle_direction);
        look_at.x += CHANGEBY * -sinf(angle_direction);
        look_at.z += CHANGEBY * cosf(angle_direction);
        break;
    case GLFW_KEY_W:
        eye_position.x += CHANGEBY * sinf(angle_direction);
        eye_position.z += CHANGEBY * -cosf(angle_direction);
        look_at.x += CHANGEBY * sinf(angle_direction);
        look_at.z += CHANGEBY * -cosf(angle_direction);
        break;
    }
}

glm::mat4x4 MovingCamera::get_view_matrix() const {
    return glm::lookAt(eye_position, look_at, glm::vec3(0.0f, 1.0f, 0.0f));
}

const float SelectedCamera::max_distance = 10.0f;

void SelectedCamera::update_eye_pos(){
    if (switch_axes) {
        eye_position.x = distance * cosf(angle_elevation) * sinf(angle_direction);
        eye_position.z = distance * sinf(angle_elevation);
        eye_position.y = distance * cosf(angle_elevation) * cosf(angle_direction);
    } else {
        eye_position.x = distance * cosf(angle_elevation) * -sinf(angle_direction);
        eye_position.y = distance * sinf(angle_elevation);
        eye_position.z = distance * cosf(angle_elevation) * cosf(angle_direction);
    }
}

void SelectedCamera::on_mouse_move(double x, double y){
    float dx = static_cast<float>(x - last_x);
    float dy = static_cast<float>(y - last_y);
    last_x = static_cast<int>(x);
    last_y = static_cast<int>(y);

    if (is_rotating) {
        angle_direction += dx * angle_sensitivity;
        angle_elevation += dy * angle_sensitivity;

        // Clamps the results.
        angle_elevation = glm::clamp(angle_elevation, 0.2f, 1.5f);
    }
    if (is_rotating){ update_eye_pos(); }
}

void SelectedCamera::move(int key, int action) {
    switch (key)
    {
    case GLFW_KEY_A:
        eye_position.x += CHANGEBY * -cosf(angle_direction);
        eye_position.z += CHANGEBY * -sinf(angle_direction);
        break;
    case GLFW_KEY_D:
        eye_position.x += CHANGEBY * cosf(angle_direction);
        eye_position.z += CHANGEBY * sinf(angle_direction);
        break;
    case GLFW_KEY_S:
        eye_position.x += CHANGEBY * -sinf(angle_direction);
        eye_position.z += CHANGEBY * cosf(angle_direction);
        break;
    case GLFW_KEY_W:
        eye_position.x += CHANGEBY * sinf(angle_direction);
        eye_position.z += CHANGEBY * -cosf(angle_direction);
        break;
    }
}

glm::mat4x4 SelectedCamera::get_view_matrix() const {
    return glm::lookAt(eye_position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}