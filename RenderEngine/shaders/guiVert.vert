#version 450

layout(location = 0) in vec2 positions;
layout(location = 1) in vec2 texCoords;

layout(location = 0) out vec2 outTextCoords;

layout(binding = 0, std140) uniform UniformBufferObject 
{
	mat4 projM;
	mat4 modelM;

	uint strLenght;
	uint packedstrData[10];

} ubo;

void main() 
{
	outTextCoords = texCoords;
	gl_Position = ubo.projM * ubo.modelM * vec4(positions, 0.0, 1.0);
}

