#include "pch.h"
#include "Core/Skeleton.h"

#include "Core/ECS/TransformComponent.h"
#include "Utils/MathUtil.h"

i32 BoneInfo::idCounter = -1;

BoneInfo::BoneInfo():
	name(""),
	id(++idCounter),
	offsetMatrix(1)
{

}

BoneInfo::~BoneInfo()
{

}


u32 Skeleton::idCounter = 0;

Skeleton::Skeleton():
	id(++idCounter),
	boneInfos()
{

}

Skeleton::~Skeleton()
{

}

void Skeleton::addBone(const BoneInfo bone)
{
	boneInfos.emplace(std::make_pair(bone.name.c_str(), bone));
}

void Skeleton::generateBoneUniform()
{
	for (auto bone : boneInfos)
	{
		std::string boneName = bone.first;
		BoneInfo& boneInfo = bone.second;

		boneUniform.boneMatrices[boneInfo.id] = boneInfo.offsetMatrix;
	}
}

void Skeleton::updateBoneUniform(Entity* rootEntity)
{
	calculateBoneTransform(rootEntity);
	boneUniformUpdated();
}

void Skeleton::calculateBoneTransform(Entity* entity)
{
	if (boneInfos.find(entity->name.c_str()) != boneInfos.end())
	{
		TransformComponent* transComp = entity->GetComponent<TransformComponent>();
		mat4 transformation = transComp->getGlobalTransformation();

		BoneInfo& boneInfo = boneInfos[entity->name.c_str()];
		boneUniform.boneMatrices[boneInfo.id] = transformation * boneInfo.offsetMatrix;
	}

	for (u32 i = 0; i < entity->children.size(); i++)
	{
		Entity* childEntity = entity->children[i];
		calculateBoneTransform(childEntity);
	}
}