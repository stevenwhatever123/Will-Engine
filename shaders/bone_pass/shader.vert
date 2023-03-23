#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in ivec4 boneIds;
layout(location = 5) in vec4 weights;

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

		// A optimised way to calculate a transformed normal without using inverse transpose
		// Reference: https://lxjk.github.io/2017/10/01/Stop-Using-Normal-Matrix.html
		mat3 upperMatrix = mat3(boneMatrices[boneIds[i]]);
		float scaleX = length(upperMatrix[0]);
		float scaleY = length(upperMatrix[1]);
		float scaleZ = length(upperMatrix[2]);
		vec3 scale = vec3(scaleX, scaleY, scaleZ);

		vec4 localNormal = normalize(vec4(upperMatrix * (normal / scale), 0));
		vec4 localTangent = normalize(modelTransformation * vec4(tangent, 1));
		vec4 localBitangent = normalize(vec4(cross(localTangent.rgb, localNormal.rgb), 0));

		finalPosition += localPosition * weights[i];
		finalNormal += localNormal * weights[i];
		finalTangent += localTangent * weights[i];
		finalBitangent += localBitangent * weights[i];
	}

	oPosition = finalPosition;
	oNormal = normalize(finalNormal);
	oTangent = normalize(finalTangent);
	oBitangent = normalize(finalBitangent);
	oTexCoord = texCoord;

	//gl_Position = projectMatrix * cameraMatrix * modelTransformation * finalPosition;
	gl_Position = projectMatrix * cameraMatrix * finalPosition;
}