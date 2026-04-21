#include "shader_program.hpp"

class BaseTexShader : public ShaderProgram
{
public:
    GLuint mvpULoc;
protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};

class SphereShader : public ShaderProgram
{
public:
    GLuint mvpULoc;
    GLuint colorULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};