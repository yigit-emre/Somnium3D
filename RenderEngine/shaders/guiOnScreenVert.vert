#version 450

layout(location = 0) in vec2 positions;
layout(location = 1) in vec3 colors;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    vec2 projection;
} pushConstants;


void main() 
{
	if(colors.r == 0.0 && colors.g == 0.0 && colors.b == 0.0) 
		outColor = vec4(colors, 0.0);
	else
		outColor = vec4(colors, 1.0);
	gl_Position = vec4(positions * pushConstants.projection, 0.0, 1.0);
}