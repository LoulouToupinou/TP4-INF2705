#version 330 core

layout(location = 0) in vec3 vertex;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec2 in_texCoords;

out vec2 texCoords;
out vec3 normal;
out vec3 vertexToLight;
out vec3 vertexToCamera;

uniform mat4 mvp;
uniform mat4 model;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;

void main()
{
    texCoords = in_texCoords;
    normal = mat3(transpose(inverse(model))) * in_normal;
    vec3 modelVertex = vec3(model * vec4(vertex, 1.0));
    vertexToLight = lightPosition - modelVertex;
    vertexToCamera = cameraPosition - modelVertex;
	gl_Position = mvp * vec4(vertex, 1.0);
}
