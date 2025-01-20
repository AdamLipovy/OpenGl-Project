#include "ObjectRenderStorage.hpp"

namespace ORS{
    void ORS::bind(){
        for (size_t i = 0; i < buffers.size(); i++)
        {
            BufferData curr = buffers[i];
            switch(curr.type){
                case GL_VERTEX_ARRAY:
                    glBindVertexArray(curr.adress);
                    break;

                default:
                    glBindBufferBase(curr.type, curr.binding, curr.adress);
            }
        }
    }

    ORS::ORS(ArrayData* arrayData, GLuint* program = 0){
        this->program = Option<GLuint>(program);
        geometry = Option<Geometry>();
        texture = Option<TextureData>();
        drawArrays = Option<ArrayData>(arrayData);
    }

    ORS::ORS(Geometry* geometryData, TextureData* textureData, GLuint* program = 0){
        this->program = Option<GLuint>(program);
        geometry = Option<Geometry>(geometryData);
        texture = Option<TextureData>(textureData);
        drawArrays = Option<ArrayData>();
    }

    void ORS::AddBuffer(BufferData newBuff) {buffers.push_back(newBuff);}
    void ORS::AddBuffer(GLsizei type , GLsizei binding, GLsizei adress){buffers.push_back(BufferData(type, binding, adress));}
    void ORS::AddBuffer(BufferData* newBuff, size_t buffCount){
        for (size_t i = 0; i < buffCount; i++){
            buffers.push_back(newBuff[i]);
        }
    }

    void ORS::SetBuffers(BufferData* newBuff, size_t buffCount){
        buffers.clear();
        AddBuffer(newBuff, buffCount);
    }

    void ORS::render(){
        if(program.toggle) glUseProgram(*(program.value));
        bind();
        if(texture.toggle){
            if(program.toggle) glUniform1i(glGetUniformLocation(*(program.value), "has_texture"), true);
            glBindTextureUnit(texture.value->adress, texture.value->texture);
        }
        else if(drawArrays.toggle){
            ArrayData ptr = *(drawArrays.value);
            glDrawArrays(ptr.type, ptr.offset, ptr.point_count);
        }
        else if(geometry.toggle){
            geometry.value->draw();
        }
    }



    ORS_instanced::ORS_instanced(ArrayData* arrayData, GLuint* program = 0) : ORS(arrayData, program){ object_count = 0; }
    ORS_instanced::ORS_instanced(Geometry* geometryData, TextureData* textureData, GLuint* program = 0) : ORS(geometryData, textureData, program){ object_count = 0;}

    void ORS_instanced::render(){
        if(program.toggle) glUseProgram(*(program.value));
        bind();
        if(texture.toggle){
            if(program.toggle) glUniform1i(glGetUniformLocation(*(program.value), "has_texture"), true);
            glBindTextureUnit(texture.value->adress, texture.value->texture);
        }
        else if(drawArrays.toggle){
            ArrayData ptr = *(drawArrays.value);
            glDrawArraysInstanced(ptr.type, ptr.offset, ptr.point_count, object_count);
        }
        //TODO instancing for geometries
        else if(geometry.toggle){
            geometry.value->draw();
        }
    }
}