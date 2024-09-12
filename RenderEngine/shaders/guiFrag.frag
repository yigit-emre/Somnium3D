#version 450

layout(location = 0) in vec2 outTextCoords;
layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D textureSampler;

void main() 
{
	FragColor = vec4(1.0, 1.0, 1.0, 1.0) * texture(textureSampler, outTextCoords).r;
}