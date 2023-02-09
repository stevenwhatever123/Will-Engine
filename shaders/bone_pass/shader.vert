#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in ivec4 boneIds;
layout(location = 4) in vec4 weights;

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
layout(location = 2) out vec2 oTexCoord;

void main()
{
	vec4 finalPosition = vec4(0);
	vec4 finalNormal = vec4(0);
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
		vec4 localNormal = normalize(boneMatrices[boneIds[i]] * vec4(normal, 1));

		finalPosition += localPosition * weights[i];
		finalNormal += localNormal * weights[i];
	}

	//oPosition = modelTransformation * finalPosition;
	oPosition = finalPosition;
	oNormal = normalize(finalNormal);
	oTexCoord = texCoord;

	//gl_Position = projectMatrix * cameraMatrix * modelTransformation * finalPosition;
	gl_Position = projectMatrix * cameraMatrix * finalPosition;
}