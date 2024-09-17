#version 450

layout(location = 0) out vec4 FragColor;
layout (input_attachment_index = 0, binding = 0) uniform subpassInput offScreenInput;

void main() 
{
	FragColor = subpassLoad(offScreenInput);
}