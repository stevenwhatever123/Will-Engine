#version 450 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 texCoord;

layout(set = 1, binding = 0) uniform sampler2D texColor;

layout(location = 0) out vec4 oColor;

void main()
{
	float mipmapLevel = textureQueryLod(texColor, texCoord).x;

	oColor = textureLod(texColor, texCoord, mipmapLevel);
	//oColor = texture(texColor, texCoord);
}