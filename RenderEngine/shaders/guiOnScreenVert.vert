#version 460

layout(location = 0) in vec2 positions;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 colors;

layout(location = 0) out vec3 color;
layout(location = 1) out vec2 texCoord;

layout(push_constant) uniform PushConstants {
    vec2 projection;
} pushConstants;


void main() 
{
	color = colors;
	texCoord = texCoords;
	gl_Position = vec4(positions * pushConstants.projection - vec2(1.0, 1.0), 0.0, 1.0);
}