#version 450 

layout(location = 0) in vec2 outTextCoords;
layout(location = 0) out vec4 FragColor;

//push constant for widgetTypeIndex

layout(binding = 1) uniform sampler2D textureSampler;

void main() 
{
	FragColor = texture(textureSampler, outTextCoords) + vec4(0.0, 0.0, 0.0, 1.0);
}