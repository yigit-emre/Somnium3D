#version 450

layout(location = 0) in vec2 outTextCoords;
layout(location = 0) out vec4 FragColor;

layout(binding = 1) uniform sampler2D textureSampler;

layout(binding = 0, std140) uniform UniformBufferObject 
{
	mat4 projM;
	mat4 modelM;

	uint strLenght;
	uint packedstrData[10];
} ubo;



vec4 fontRendering() 
{
	uint charIndex = uint(outTextCoords.x * float(ubo.strLenght));
	uint packedIndex = uint(charIndex / 2);
	vec2 texCoords = vec2(outTextCoords.x - charIndex / ubo.strLenght, outTextCoords.y);
	texCoords += vec2(float(((ubo.packedstrData[packedIndex] >> (charIndex % 2) * 16) & 0xFF)), float(((ubo.packedstrData[packedIndex] >> (charIndex % 2) * 16 + 8) & 0xFF)));
	return vec4(1.0, 1.0, 1.0, 1.0) * texture(textureSampler,  texCoords).r;
}

void main() 
{
	if(ubo.strLenght > 0) 
	{
		FragColor = fontRendering();
	}
	else 
	{
		FragColor = vec4(0.0, 1.0, 1.0, 1.0);
	}
}