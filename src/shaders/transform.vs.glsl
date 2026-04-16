#version 330 core

layout(location = 0) in vec3 vertex;
layout(location = 3) in vec2 texCoords;

out vec2 fragTexCoords;

uniform mat4 mvp;

void main()
{
    fragTexCoords = texCoords;
	gl_Position = mvp * vec4(vertex, 1.0);
}
