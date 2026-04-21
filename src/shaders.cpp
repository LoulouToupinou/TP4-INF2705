#include "shaders.hpp"

void BezierShader::load()
{
    const char* VERTEX_SRC_PATH = "./shaders/bezier.vs.glsl";
    const char* FRAGMENT_SRC_PATH = "./shaders/bezier.fs.glsl";

    name_ = "BezierShader";
    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();

}

void BezierShader::getAllUniformLocations()
{
    mvpULoc = glGetUniformLocation(id_, "mvp");
}

void SphereShader::load()
{
    const char* VERTEX_SRC_PATH = "./shaders/sphere.vs.glsl";
    const char* FRAGMENT_SRC_PATH = "./shaders/sphere.fs.glsl";
    
    name_ = "SphereShader";
    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();
}

void SphereShader::getAllUniformLocations()
{
    mvpULoc = glGetUniformLocation(id_, "mvp");
    colorULoc = glGetUniformLocation(id_, "color");
}
