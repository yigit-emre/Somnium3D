#version 460

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec3 color;
layout(location = 1) in vec2 texCoord;

layout(binding = 0) uniform sampler2D textureSampler;

void main() 
{
	FragColor = vec4(color, texture(textureSampler, texCoord).r);
}