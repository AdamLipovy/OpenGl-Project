
#pragma once

#include "camera.h"
#include "glm_headers.hpp"
#include <algorithm>

class MovingCamera : public Camera{
private:
    void update_eye_pos();

protected:
    static const float max_distance;

public:
    
    glm::vec3 look_at = glm::vec3(0.0f, 0.0f, 0.0f);

    MovingCamera(bool switch_axis = false) : Camera(){
        eye_position = glm::vec3(3.0f, 5.0f, 3.0f);
        update_eye_pos();
        angle_elevation = 0.5f;
        update_eye_pos();
    }

    void move(int key, int action);
    void on_mouse_move(double x, double y);

    glm::mat4x4 get_view_matrix() const;
};

class SelectedCamera : public Camera{
private:
    void update_eye_pos();

protected:
    static const float max_distance;

public:
    SelectedCamera(bool switch_axis = false) : Camera(){
        eye_position = glm::vec3(3.0f, 5.0f, 3.0f);
        distance = 1.5f;
        angle_elevation = 0.5f;
        update_eye_pos();
    }

    void move(int key, int action);
    void on_mouse_move(double x, double y);

    glm::mat4x4 get_view_matrix() const;
};

