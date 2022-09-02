#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec3 oNormal;
layout(location = 1) out vec2 oTexCoord;

void main()
{
	oNormal = normal;
	oTexCoord = texCoord;

	mat4 transformation;
	transformation[0][0] = 1;
	transformation[1][1] = 1;
	transformation[2][2] = 1;
	transformation[3] = vec4(0, 0, -50, 0);

	gl_Position = transformation * vec4(position, 1);
}