#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 outColor;

layout(binding = 0) uniform sampler2D textureSampler;

void main() 
{
	FragColor = outColor;
}