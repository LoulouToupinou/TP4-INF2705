#include "shader_program.hpp"

class BaseTexShader : public ShaderProgram
{
public:
    GLuint mvpULoc;
protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};