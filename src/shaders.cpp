#include "shaders.hpp"

void BaseTexShader::load()
{
    const char* VERTEX_SRC_PATH = "./shaders/transform.vs.glsl";
    const char* FRAGMENT_SRC_PATH = "./shaders/transform.fs.glsl";

    name_ = "BaseTexShader";
    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();

}

void BaseTexShader::getAllUniformLocations()
{
    mvpULoc = glGetUniformLocation(id_, "mvp");
}
