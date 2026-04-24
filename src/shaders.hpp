#include "shader_program.hpp"

struct Material
{
    GLfloat kAmbient;
    GLfloat kDiffuse;
    GLfloat kSpecular;
    GLfloat shininess;
};

class BezierShader : public ShaderProgram
{
public:
    GLuint mvpULoc;
    GLuint colorULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};

class PhongShader : public ShaderProgram
{
public:
    GLuint mULoc;
    GLuint mvpULoc;
    GLuint lightColorULoc;
    GLuint lightPositionULoc;
    GLuint cameraPositionULoc;
    GLuint kAmbientULoc;
    GLuint isLightSourceULoc;
    void setMaterial(const Material material);

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;

private:
    GLuint kDiffuseULoc;
    GLuint kSpecularULoc;
    GLuint shininessULoc;
};

class BloomShader : public ShaderProgram
{
public:
    GLuint horizontalULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};