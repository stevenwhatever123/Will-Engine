#version 450 core
//#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 texCoord;

layout(set = 1, binding = 0) uniform sampler2D texColor;

layout(location = 0) out vec4 oColor;

void main()
{
	//oColor = vec4(1, 1, 1, 1);
	oColor = vec4(texCoord, 0, 1);
}