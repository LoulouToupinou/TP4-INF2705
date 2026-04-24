#version 330 core

layout(location = 0) in vec3 vertex;
layout(location = 3) in vec2 texCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = texCoords;
    gl_Position = vec4(vertex, 1.0);
}