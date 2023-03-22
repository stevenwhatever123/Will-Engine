#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 texCoord;

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
layout(location = 2) out vec4 oTangent;
layout(location = 3) out vec4 oBitangent;
layout(location = 4) out vec2 oTexCoord;

void main()
{
	oPosition = modelTransformation * vec4(position, 1);

	mat3 upperMatrix = mat3(modelTransformation);
	mat3 normalMatrix = transpose(upperMatrix);

	oNormal = normalize(vec4(normalMatrix * normal, 0));
	oTangent = normalize(vec4(normalMatrix * tangent, 0));
	oBitangent = normalize(vec4(cross(oTangent.rgb, oNormal.rgb), 0));
	oTexCoord = texCoord;

	gl_Position = projectMatrix * cameraMatrix * modelTransformation * vec4(position, 1);
}