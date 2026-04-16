#pragma once

#include <glbinding/gl/gl.h>

using namespace gl;

class Model
{
public:
    void load(const char* path);
    
    ~Model();
    
    void draw();
    void load(const float* vertexData, size_t vertexDataSize, const unsigned int* elementData, size_t elementDataSize);


private:
    GLuint vao_, vbo_, ebo_;
    GLsizei count_;
};

