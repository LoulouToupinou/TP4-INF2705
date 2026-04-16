#include "model.hpp"

#include "happly.h"

using namespace gl;

struct Pos
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct Color
{
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
};

struct Vertex 
{
    Pos pos;
    Color color;
};

struct PositionAttribute
{
    float x, y, z;
};

struct ColorUCharAttribute
{
    unsigned char r, g, b;
};

struct NormalAttribute
{
    float x, y, z;
};

struct TexCoordAttribute
{
    float s, t;
};

struct VertexModel
{
    PositionAttribute pos;
    ColorUCharAttribute color;
    NormalAttribute normal;
    TexCoordAttribute texCoord;
};

const GLuint VERTEX_POSITION_INDEX = 0;
const GLuint VERTEX_COLOR_INDEX = 1;
const GLuint VERTEX_NORMAL_INDEX = 2;
const GLuint VERTEX_TEXCOORDS_INDEX = 3;


void Model::load(const char* path)
{
    happly::PLYData plyIn(path);

    happly::Element& vertex = plyIn.getElement("vertex");
    std::vector<float> positionX = vertex.getProperty<float>("x");
    std::vector<float> positionY = vertex.getProperty<float>("y");
    std::vector<float> positionZ = vertex.getProperty<float>("z");

    std::vector<float> normalX, normalY, normalZ;
    try
    {
        normalX = vertex.getProperty<float>("nx");
        normalY = vertex.getProperty<float>("ny");
        normalZ = vertex.getProperty<float>("nz");
    }
    catch (std::runtime_error& e)
    {
        std::cout << "No normal attribute for model \"" << path << "\"" << std::endl;
    }
    
    std::vector<unsigned char> colorRed, colorGreen, colorBlue;
    try
    {
        colorRed   = vertex.getProperty<unsigned char>("red");
        colorGreen = vertex.getProperty<unsigned char>("green");
        colorBlue  = vertex.getProperty<unsigned char>("blue");
    }
    catch (std::runtime_error& e)
    {
        std::cout << "No color attribute for model \"" << path << "\"" << std::endl;
    }

    std::vector<float> texCoordsX, texCoordsY;
    try
    {
        texCoordsX = vertex.getProperty<float>("s");
        texCoordsY = vertex.getProperty<float>("t");
    }
    catch (std::runtime_error& e)
    {
        std::cout << "No texture coordinate attribute for model \"" << path << "\"" << std::endl;
    }

    std::vector<std::vector<unsigned int>> facesIndices = plyIn.getFaceIndices<unsigned int>();
    
    std::vector<VertexModel> vPos(positionX.size());
    for (size_t i = 0; i < vPos.size(); i++)
    {
        vPos[i] = {0};
    
        vPos[i].pos.x = positionX[i];
        vPos[i].pos.y = positionY[i];
        vPos[i].pos.z = positionZ[i];

        if (!colorRed.empty()) {
            vPos[i].color.r = colorRed[i];
            vPos[i].color.g = colorGreen[i];
            vPos[i].color.b = colorBlue[i];
        } else {
            vPos[i].color.r = 255;
            vPos[i].color.g = 255;
            vPos[i].color.b = 255;
        }

        if (!normalX.empty())
        {
            vPos[i].normal.x = normalX[i];
            vPos[i].normal.y = normalY[i];
            vPos[i].normal.z = normalZ[i];
        }
        
        if (!texCoordsX.empty())
        {
            vPos[i].texCoord.s = texCoordsX[i];
            vPos[i].texCoord.t = texCoordsY[i];
        }
    }
    
    std::vector<unsigned int> elementsData(facesIndices.size() * 3);        
    for (size_t i = 0; i < facesIndices.size(); i++)
    {
        for (size_t j = 0; j < facesIndices[i].size(); j++)
        {
            elementsData[3*i+j] = facesIndices[i][j];
        }
    }
    
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(VertexModel), &vPos[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementsData.size() * sizeof(unsigned int), &elementsData[0], GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);    
    
    glEnableVertexAttribArray(VERTEX_POSITION_INDEX);
    glVertexAttribPointer(VERTEX_POSITION_INDEX, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (GLvoid*)(offsetof(VertexModel, pos)));        
    
    glEnableVertexAttribArray(VERTEX_COLOR_INDEX);
    glVertexAttribPointer(VERTEX_COLOR_INDEX, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexModel), (GLvoid*)(offsetof(VertexModel, color)));
    
    if (!normalX.empty())
    {
        glEnableVertexAttribArray(VERTEX_NORMAL_INDEX);
        glVertexAttribPointer(VERTEX_NORMAL_INDEX, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (GLvoid*)(offsetof(VertexModel, normal)));
    }
    else
        glDisableVertexAttribArray(VERTEX_NORMAL_INDEX);
        
    if (!texCoordsX.empty())
    {
        glEnableVertexAttribArray(VERTEX_TEXCOORDS_INDEX);
        glVertexAttribPointer(VERTEX_TEXCOORDS_INDEX, 2, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (GLvoid*)(offsetof(VertexModel, texCoord)));
    }
    else
        glDisableVertexAttribArray(VERTEX_TEXCOORDS_INDEX);
    
    glBindVertexArray(0);
    
    count_ = elementsData.size();
}

void Model::load(const float* vertexData, size_t vertexDataSize, const unsigned int* elementData, size_t elementDataSize)
{
    size_t nVertices = vertexDataSize / (5 * sizeof(float));
    std::vector<VertexModel> vPos(nVertices);
    for (size_t i = 0; i < nVertices; i++)
    {
        vPos[i] = {0};
    
        vPos[i].pos.x = vertexData[i*5 + 0];
        vPos[i].pos.y = vertexData[i*5 + 1];
        vPos[i].pos.z = vertexData[i*5 + 2];

        vPos[i].color.r = 255;
        vPos[i].color.g = 255;
        vPos[i].color.b = 255;

        vPos[i].normal.x = 0.0f;
        vPos[i].normal.y = 1.0f;
        vPos[i].normal.z = 0.0f;
        
        vPos[i].texCoord.s = vertexData[i*5 + 3];
        vPos[i].texCoord.t = vertexData[i*5 + 4];
    }
    
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(VertexModel), &vPos[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementDataSize, elementData, GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);    
    
    glEnableVertexAttribArray(VERTEX_POSITION_INDEX);
    glVertexAttribPointer(VERTEX_POSITION_INDEX, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (GLvoid*)(offsetof(VertexModel, pos)));        
    
    glEnableVertexAttribArray(VERTEX_COLOR_INDEX);
    glVertexAttribPointer(VERTEX_COLOR_INDEX, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexModel), (GLvoid*)(offsetof(VertexModel, color)));

    glEnableVertexAttribArray(VERTEX_NORMAL_INDEX);
    glVertexAttribPointer(VERTEX_NORMAL_INDEX, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (GLvoid*)(offsetof(VertexModel, normal)));
        
    glEnableVertexAttribArray(VERTEX_TEXCOORDS_INDEX);
    glVertexAttribPointer(VERTEX_TEXCOORDS_INDEX, 2, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (GLvoid*)(offsetof(VertexModel, texCoord)));
    
    glBindVertexArray(0);
    
    count_ = elementDataSize / sizeof(unsigned int);
}

Model::~Model()
{
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
}

void Model::draw()
{
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, count_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
