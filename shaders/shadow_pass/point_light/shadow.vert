#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 texCoord;

layout(push_constant) uniform modelMatrices
{
	mat4 modelTransformation;
};

void main()
{
	gl_Position = modelTransformation * vec4(position, 1);
}