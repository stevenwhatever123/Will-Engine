#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 texCoord;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

layout(set = 0, binding = 0) uniform sceneMatrix
{
	mat4 cameraMatrix;
	mat4 projectMatrix;
};

const uint MAX_BONES = 256;
const uint MAX_BONE_INFLUENCE = 4;
layout(set = 2, binding = 2) uniform boneMatricesInfo
{
	mat4 boneMatrices[MAX_BONES];
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
	vec4 finalPosition = vec4(0);
	vec4 finalNormal = vec4(0);
	vec4 finalTangent = vec4(0);
	vec4 finalBitangent = vec4(0);
	for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
	{
		if(boneIds[i] < 0)
			continue;
		if(boneIds[i] >= MAX_BONES)
		{
			finalPosition = vec4(position, 1);
			break;
		}

		vec4 localPosition = boneMatrices[boneIds[i]] * vec4(position, 1);

		mat3 upperMatrix = mat3(boneMatrices[boneIds[i]]);
		mat3 normalMatrix = transpose(upperMatrix);

		vec4 localNormal = normalize(vec4(upperMatrix * normal, 1));
		//vec4 localTangent = normalize(vec4(upperMatrix * tangent, 1));
		//vec4 localBitangent = normalize(vec4(upperMatrix * bitangent, 1));
		vec4 localTangent = normalize(boneMatrices[boneIds[i]] * vec4(tangent, 1));
		vec4 localBitangent = normalize(boneMatrices[boneIds[i]] * vec4(bitangent, 1));

		finalPosition += localPosition * weights[i];
		finalNormal += localNormal * weights[i];
		finalTangent += localTangent * weights[i];
		finalBitangent += localBitangent * weights[i];
	}

	//oPosition = modelTransformation * finalPosition;
	oPosition = finalPosition;
	oNormal = normalize(finalNormal);
	oTangent = normalize(finalTangent);
	oBitangent = normalize(finalBitangent);
	oTexCoord = texCoord;

	//gl_Position = projectMatrix * cameraMatrix * modelTransformation * finalPosition;
	gl_Position = projectMatrix * cameraMatrix * finalPosition;
}