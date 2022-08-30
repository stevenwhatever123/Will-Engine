#version 450 core

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 texCoord;


layout(location = 0) out vec4 oColor;

void main()
{
	oColor = vec4(1, 1, 1, 1);
}