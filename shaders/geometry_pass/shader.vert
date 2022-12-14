#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(set = 0, binding = 0) uniform sceneMatrix
{
	mat4 cameraMatrix;
	mat4 projectMatrix;
};

layout(push_constant) uniform modelMatrix
{
	mat4 modelTransformation;
};

layout(location = 0) out vec4 oPosition;
layout(location = 1) out vec4 oNormal;
layout(location = 2) out vec2 oTexCoord;

void main()
{
	oPosition = modelTransformation * vec4(position, 1);
	oNormal = vec4(normal, 1);
	oTexCoord = texCoord;

	gl_Position = projectMatrix * cameraMatrix * modelTransformation * vec4(position, 1);
}