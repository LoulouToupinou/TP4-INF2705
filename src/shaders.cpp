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

void PhongShader::load()
{
    const char* VERTEX_SRC_PATH = "./shaders/phong.vs.glsl";
    const char* FRAGMENT_SRC_PATH = "./shaders/phong.fs.glsl";
    
    name_ = "PhongShader";
    loadShaderSource(GL_VERTEX_SHADER, VERTEX_SRC_PATH);
    loadShaderSource(GL_FRAGMENT_SHADER, FRAGMENT_SRC_PATH);
    link();
}

void PhongShader::getAllUniformLocations()
{
    mvpULoc = glGetUniformLocation(id_, "mvp");
    mULoc = glGetUniformLocation(id_, "model");
    lightPositionULoc = glGetUniformLocation(id_, "lightPosition");
    cameraPositionULoc = glGetUniformLocation(id_, "cameraPosition");
    kAmbientULoc = glGetUniformLocation(id_, "kAmbient");
    kDiffuseULoc = glGetUniformLocation(id_, "kDiffuse");
    kSpecularULoc = glGetUniformLocation(id_, "kSpecular");
    shininessULoc = glGetUniformLocation(id_, "shininess");
    lightColorULoc = glGetUniformLocation(id_, "lightColor");
    isLightSourceULoc = glGetUniformLocation(id_, "isLightSource");
}

void PhongShader::setMaterial(const Material material)
{
    glUniform1f(kAmbientULoc, material.kAmbient);
    glUniform1f(kDiffuseULoc, material.kDiffuse);
    glUniform1f(kSpecularULoc, material.kSpecular);
    glUniform1f(shininessULoc, material.shininess);
}