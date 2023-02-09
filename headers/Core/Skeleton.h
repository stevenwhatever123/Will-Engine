#pragma once
#include "UniformClass.h"

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
	std::map<std::string, BoneInfo> boneInfos;
	BoneUniform boneUniform;

	bool uniformUpdated;

public:

	// For Vulkan Uniform Buffer
	VulkanDescriptorSet boneDescriptorSet;
	VulkanAllocatedMemory boneUniformBuffer;

public:

	Skeleton();
	~Skeleton();

	void addBone(const BoneInfo bone);

	void boneUniformReset() { uniformUpdated = false; };
	void boneUniformUpdated() { uniformUpdated = true; };
	bool hasUniformUpdated() const { return uniformUpdated; };

	void generateBoneUniform();
	void updateBoneUniform(Entity* rootEntity);
	void calculateBoneTransform(Entity* entity);

	bool hasBones() const { return boneInfos.size(); };
	bool hasBone(std::string name) const { return boneInfos.find(name) != boneInfos.end(); };

private:

	static u32 idCounter;
};