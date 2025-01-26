#pragma once

#include "glad/glad.h"
#include "geometry.hpp"
#include <iostream>

#include <vector>
namespace ORS{
    template <typename T>
    struct Option{
        bool toggle;
        T value;

        Option(T value){ this->toggle = true; this->value = value; }
        Option(){ toggle = false; value = T();}
    };

    struct BufferData{
        GLsizei type;
        GLsizei binding;
        GLuint* adress;

        BufferData(){}
        BufferData(BufferData* in){type = in->type; binding = in->binding; adress = in->adress;}
        BufferData(GLsizei type, GLsizei binding, GLuint* adress){
            this->type = type;
            this->binding = binding;
            this->adress = adress;
        }
    };
    struct ArrayData{
        GLsizei type;
        GLint offset;
        GLsizei point_count;

        ArrayData(){}
        ArrayData(GLsizei type, GLint offset, GLsizei point_count){
            this->type = type;
            this->offset = offset;
            this->point_count = point_count;
        }
    };
    struct TextureData{
        GLuint adress;
        GLuint texture;
        TextureData(){}
        TextureData(GLuint adress, GLuint texture){this->adress = adress; this->texture = texture; }
    };
    class ORS{
    protected:
        std::vector<BufferData> buffers;
        Option<GLuint*> program;
        Option<Geometry> geometry;
        Option<TextureData> texture;
        Option<ArrayData> drawArrays;

        void bind();
    public:
        ORS(ArrayData* arrayData, GLuint* program);
        ORS(Geometry* geometryData, TextureData* textureData, GLuint* program);
        void virtual render();
        void AddBuffer(BufferData);
        void AddBuffer(GLsizei, GLsizei, GLuint*);
        void AddBuffer(BufferData*, size_t);
        GLuint* GetBufferAdress(int index);

        void SetBuffers(BufferData*, size_t);
    };

    struct DynamicStorageData{
        BufferData buffer;
        size_t max_size;
        size_t allocate_by;
        DynamicStorageData(){}
        DynamicStorageData(BufferData* buffer){this->buffer = BufferData(buffer);}
    };
    class ORS_instanced : public ORS{
    private:
        Option<DynamicStorageData> dynamic_buffer = Option<DynamicStorageData>();

    public:
        GLsizei object_count;

        ORS_instanced(ArrayData* arrayData, GLuint* program);
        ORS_instanced(Geometry* geometryData, TextureData* textureData, GLuint* program);

        void AddDynamicBuffer(BufferData*, size_t, size_t allocate_by = 10);
        GLuint* GetDynamicBufferAdress();

        void AddInstanceBuffer(BufferData*, size_t);

        void AddInstance();
    
        void render();
    };
}