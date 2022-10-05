#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(set = 0, binding = 0) uniform sceneMatrix
{
	mat4 cameraProjectionMatrix;
};

layout(push_constant) uniform modelMatrix
{
	mat4 modelTransformation;
} ;

layout(location = 0) out vec3 oNormal;
layout(location = 1) out vec2 oTexCoord;

void main()
{
	oNormal = normal;
	oTexCoord = texCoord;

	gl_Position = cameraProjectionMatrix * modelTransformation * vec4(position, 1);
}