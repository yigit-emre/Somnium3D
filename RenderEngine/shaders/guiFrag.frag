#version 450 

layout(location = 0) in vec2 outTextCoords;
layout(location = 0) out vec4 FragColor;

//push constant for widgetTypeIndex

layout(binding = 1) uniform sampler2D textureSampler;

layout(binding = 0, std140) uniform UniformBufferObject 
{
	mat4 projM;
	mat4 modelM;

	float strLenght;
	vec2 charOffsets[40];
} ubo;



void main() 
{
	if(ubo.strLenght > 0) 
	{
		uint charIndex = uint(outTextCoords * ubo.strLenght);
		FragColor = vec4(1.0, 1.0, 1.0, 1.0) * texture(textureSampler,  ubo.charOffsets[charIndex] + outTextCoords).r;
	}
	else 
	{
		FragColor = vec4(1.0, 1.0, 1.0, 1.0);
	}
}