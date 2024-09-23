#version 460

layout(location = 0) in vec2 positions;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 colors;

layout(location = 0) out vec4 color;
layout(location = 1) out vec2 texCoord;

layout(push_constant) uniform PushConstants {
    vec2 projection;
} pushConstants;


void main() 
{
	if(colors.r == 0.0 && colors.g == 0.0 && colors.b == 0.0) 
		color = vec4(colors, 0.0);
	else
		color = vec4(colors, 1.0);

	texCoord = texCoords;
	gl_Position = vec4(positions * pushConstants.projection - vec2(1.0, 1.0), 0.0, 1.0);
}