#version 460

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec3 color;
layout(location = 1) in vec2 texCoord;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput offScreenInput;
layout(binding = 1) uniform sampler2D textureSampler;

void main() 
{
	vec4 offScreenColor = subpassLoad(offScreenInput);
	if(offScreenColor.a == 0)
		FragColor = vec4(color, texture(textureSampler, texCoord).r);
	else
		FragColor = offScreenColor;
}