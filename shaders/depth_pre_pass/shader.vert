#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(set = 0, binding = 0) uniform sceneMatrix
{
	mat4 cameraMatrix;
	mat4 projectMatrix;
};

layout(push_constant) uniform modelMatrices
{
	mat4 modelTransformation;
};

void main()
{
	gl_Position = projectMatrix * cameraMatrix * modelTransformation * vec4(position, 1);
}