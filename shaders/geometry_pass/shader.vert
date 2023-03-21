#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 texCoord;

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

	oNormal = normalize(vec4(upperMatrix * normal, 1));
	//oTangent = normalize(vec4(upperMatrix * tangent, 1));
	//oBitangent = normalize(vec4(upperMatrix * bitangent, 1));
	oTangent = normalize(modelTransformation * vec4(tangent, 1));
	//oBitangent = normalize(modelTransformation * vec4(bitangent, 1));
	oBitangent = vec4(cross(oTangent.rgb, oNormal.rgb), 1);
	oTexCoord = texCoord;

	gl_Position = projectMatrix * cameraMatrix * modelTransformation * vec4(position, 1);
}