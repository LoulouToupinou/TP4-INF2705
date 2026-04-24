#version 330 core

in vec2 texCoords;
in vec3 normal;
in vec3 vertexToLight;
in vec3 vertexToCamera;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 bloomColor;  

uniform sampler2D textureSampler;

uniform bool isLightSource;
uniform vec3 lightColor;

uniform float kAmbient;
uniform float kDiffuse;
uniform float kSpecular;
uniform float shininess;

void main()
{
    if (isLightSource)
    {
        color = vec4(lightColor, 1.0);
        bloomColor = color;
    }
    else
    {
        float d = length(vertexToLight);
        vec3 N = normalize(normal);
        vec3 L = normalize(vertexToLight);
        vec3 V = normalize(vertexToCamera);
        vec3 B = normalize(L + V);
        float distanceFactor = 1 / (1 + 0.1 * d + 0.01 * d* d);
        float angleLN = max(0.0, dot(L, N));
        float angleNB = max(0.0, dot(N, B));
        vec4 texColor = texture(textureSampler, texCoords);
        vec3 baseColor = kAmbient * vec3(texColor);
        vec3 lightColorDistance = distanceFactor * lightColor;
        vec3 diffuseColor = lightColorDistance * kDiffuse * angleLN;
        vec3 specularColor = lightColorDistance * kSpecular * pow(angleNB, shininess);
        color = vec4(baseColor + diffuseColor + specularColor, 1.0);
        bloomColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
