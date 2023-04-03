#pragma once
#include "UniformClass.h"

#include "Core/Animation.h"

#include "Core/Vulkan/VulkanDescriptorSet.h"
#include "Core/Vulkan/VulkanAllocatedObject.h"

#include "Core/ECS/Entity.h"

using namespace WillEngine;

class BoneInfo
{
public:

	std::string name;
	const i32 id;
	mat4 offsetMatrix;

public:

	BoneInfo();
	~BoneInfo();

	void setName(const std::string name) { this->name = name; };
	void setOffsetMatrix(const mat4 matrix) { this->offsetMatrix = matrix; };

	static void beginCreation() { idCounter = -1; };
	static void endCreation() { idCounter = -1; };

private:

	static i32 idCounter;
};

class Skeleton
{
public:

	const u32 id;
	// Order: Entity(By Name)->Bone Info
	std::unordered_map<std::string, BoneInfo> boneInfos;
	BoneUniform boneUniform;

	// Order: Entity(By Name)->Update Transform
	// A map to record whether which entity should recalculate its global world transformation
	// For more details see (Bones section): https://assimp.sourceforge.net/lib_html/data.html
	std::unordered_map<std::string, bool> necessityMap;

public:

	// For Vulkan Uniform Buffer
	VulkanDescriptorSet boneDescriptorSet;
	VulkanAllocatedMemory boneUniformBuffer;

private:

	static u32 idCounter;

public:

	Skeleton();
	~Skeleton();

	void addBone(const BoneInfo bone);

	void generateBoneUniform();

	void updateBoneUniform(Entity* rootEntity);
	void calculateBoneTransform(Entity* entity);

	bool hasBones() const { return boneInfos.size(); };
	bool hasBone(std::string name) const { return boneInfos.contains(name); };

	void generateNecessityMap(Entity* rootEntity);
	void resetNecessityMap();
	void addToNecessityMap(Entity* entity);
	void buildNecessityMap(Entity* entity);
	const std::unordered_map<std::string, bool>& getNecessityMap() const { return necessityMap; }

private:
	// This traverse all the way back to the root node
	void traverseRootNecessityMapUpdate(Entity* entity);
	// This traverse all child node
	void traverseChildNecessityMapUpdate(Entity* entity);
};