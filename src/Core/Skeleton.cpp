#include "pch.h"
#include "Core/Skeleton.h"

#include "Core/ECS/TransformComponent.h"
#include "Core/MeshComponent.h"

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
	for (auto &bone : boneInfos)
	{
		const std::string& boneName = bone.first;
		const BoneInfo& boneInfo = bone.second;

		boneUniform.boneMatrices[boneInfo.id] = boneInfo.offsetMatrix;
	}
}

void Skeleton::updateBoneUniform(Entity* rootEntity)
{
	calculateBoneTransform(rootEntity);
}

void Skeleton::calculateBoneTransform(Entity* entity)
{
	// If the bone has not been visited before, we simple ignore it and its child
	if (necessityMap.at(entity->name) == false)
		return;

	// We iterate over the node hierarchy and copy global world transformation to the uniform if necessary
	if (boneInfos.contains(entity->name))
	{
		TransformComponent* transComp = entity->GetComponent<TransformComponent>();
		const mat4& transformation = transComp->getWorldTransformation();

		BoneInfo& boneInfo = boneInfos[entity->name.c_str()];
		boneUniform.boneMatrices[boneInfo.id] = transformation * boneInfo.offsetMatrix;
	}

	for (u32 i = 0; i < entity->children.size(); i++)
	{
		Entity* childEntity = entity->children[i];

		calculateBoneTransform(childEntity);
	}
}

void Skeleton::generateNecessityMap(Entity* rootEntity)
{
	necessityMap[rootEntity->name] = false;

	for (auto* child : rootEntity->children)
	{
		generateNecessityMap(child);
	}
}

void Skeleton::resetNecessityMap()
{
	// Resetting every boolean to false
	for (auto& it : necessityMap)
	{
		it.second = false;
	}
}

void Skeleton::addToNecessityMap(Entity* entity)
{
	traverseChildNecessityMapUpdate(entity);
}

void Skeleton::buildNecessityMap(Entity* entity)
{
	traverseRootNecessityMapUpdate(entity);
}

void Skeleton::traverseRootNecessityMapUpdate(Entity* entity)
{
	if (necessityMap.contains(entity->name))
	{
		if (necessityMap.at(entity->name) == true)
			return;

		necessityMap.at(entity->name) = true;

		// Traverse the hierarchy back to the root and mark every node visited to true
		if (entity->hasParent())
		{
			Entity* parentEntity = entity->getParent();

			// We also want to make sure to include meshes that are outside the skeleton
			// by check if the current entity's has a mesh component or is a parent of the mesh component
			for (auto& child : parentEntity->children)
			{
				if (child->HasComponent<MeshComponent>() || child->ChildHasComponent<MeshComponent>())
				{
					if (!necessityMap.contains(child->name))
					{
						return;
					}

					necessityMap.at(child->name) = true;
					traverseChildNecessityMapUpdate(child);
				}
			}

			traverseRootNecessityMapUpdate(parentEntity);
		}
	}
}

void Skeleton::traverseChildNecessityMapUpdate(Entity* entity)
{
	if (necessityMap.contains(entity->name))
	{
		necessityMap.at(entity->name) = true;

		// Traverse the all child and mark every node visited to true
		for (auto& child : entity->children)
		{
			traverseChildNecessityMapUpdate(child);
		}
	}
}