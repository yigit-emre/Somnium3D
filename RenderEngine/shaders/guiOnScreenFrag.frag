#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 outColor;
layout(input_attachment_index = 0, binding = 0) uniform subpassInput offScreenInput;
layout(binding = 1) uniform sampler2D textureSampler;


void main() 
{
	FragColor = subpassLoad(offScreenInput) * (1.0 - outColor.a) + outColor;
}