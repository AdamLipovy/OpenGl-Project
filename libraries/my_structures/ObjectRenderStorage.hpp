#pragma once

#include "glad/glad.h"
#include "geometry.hpp"

#include <vector>
namespace ORS{
    template <typename T>
    struct Option{
        bool toggle;
        T* value;

        Option(T value){ this->toggle = true; this->value = &value; }
        Option(T* value){ this->toggle = true; this->value = value; }
        Option(){ toggle = false;}
    };

    struct BufferData{
        GLsizei type;
        GLsizei binding;
        GLsizei adress;

        BufferData(){}
        BufferData(BufferData* in){type = in->type; binding = in->binding; adress = in->adress;}
        BufferData(GLsizei type, GLsizei binding, GLsizei adress){
            this->type = type;
            this->binding = binding;
            this->adress = adress;
        }
    };
    struct ArrayData{
        GLsizei type;
        GLint offset;
        GLsizei point_count;

        ArrayData(GLsizei type, GLint offset, GLsizei point_count){
            this->type = type;
            this->offset = offset;
            this->point_count = point_count;
        }
    };
    struct TextureData{
        GLuint adress;
        GLuint texture;
    };
    class ORS{
    protected:
        std::vector<BufferData> buffers;
        Option<GLuint> program;
        Option<Geometry> geometry;
        Option<TextureData> texture;
        Option<ArrayData> drawArrays;

        void bind();
    public:
        ORS(ArrayData* arrayData, GLuint* program);
        ORS(Geometry* geometryData, TextureData* textureData, GLuint* program);
        void virtual render();
        void AddBuffer(BufferData);
        void AddBuffer(GLsizei, GLsizei, GLsizei);
        void AddBuffer(BufferData*, size_t);

        void SetBuffers(BufferData*, size_t);
    };
    class ORS_instanced : public ORS{
    public:
        GLsizei object_count;

        ORS_instanced(ArrayData* arrayData, GLuint* program);
        ORS_instanced(Geometry* geometryData, TextureData* textureData, GLuint* program);
        void render();
    };
}