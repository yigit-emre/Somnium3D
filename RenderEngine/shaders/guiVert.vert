#version 450

layout(location = 0) in vec2 positions;
layout(location = 1) in vec2 texCoords;

layout(location = 0) out vec2 outTextCoords;

layout(push_constant) uniform PushConstant {
	vec2 projection;
} pc;

void main() 
{
	outTextCoords = texCoords;
	gl_Position = vec4(positions * pc.projection, 0.0, 1.0);
}

