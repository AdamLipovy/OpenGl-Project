
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include <cstdlib>
#include <string>

#include "glad/glad.h"

namespace QOL{

    float pythDist(glm::vec3 a, glm::vec3 b);

    double roundClossest(double val, double mult);

    template<typename T>
    T basicTimer(double perc, T start, T end){
        return start + (end - start) * (float)perc;
    }

    float hexagonRotation(double perc, float start, float end);

    template <typename T, typename Args>
    struct FunctionType {
        T data;
        Args args;
    };

    struct RenderObject {
        glm::mat4 model_matrix;
        glm::vec4 ambient_color;
        glm::vec4 diffuse_color;
        glm::vec4 specular_color;
    };

    struct SubBufferType{
        GLuint buffer;
        size_t index;
        size_t size;
    };

    struct MatChange{
        float rotate;
        glm::vec3 rotate_around;
        glm::vec3 position;
        glm::vec3 size;
    };

    MatChange matTransition(double perc, MatChange start, MatChange end);

    template <typename T>
    void ChangeBufferSubData(FunctionType<T, SubBufferType>);

    void ChangeMatrixSubData(FunctionType<glm::mat4, SubBufferType>);
    void ChangeMatrixPositionSubData(FunctionType<glm::vec4, SubBufferType> data);
    void ChangeMatrixRotationSubData(FunctionType<float, SubBufferType>);
    void CreateMatrix(FunctionType<MatChange, SubBufferType> data);
}

namespace animation_functions{
    double ease_in_ease_out(float t);
}
