#pragma once
#define MAX_BONE_INFLUENCE 4

struct BoneWeight
{
	i32 boneIds[MAX_BONE_INFLUENCE];
	float weights[MAX_BONE_INFLUENCE];

	BoneWeight();
};