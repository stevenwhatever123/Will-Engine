#pragma once
#include <queue>

#include "Core/MeshComponent.h"
#include "Core/Material.h"
#include "Core/LightComponent.h"

#include "Core/ECS/Entity.h"

#include "Core/Vulkan/VulkanDescriptorSet.h"
#include "Core/Vulkan/VulkanFramebuffer.h"

using namespace WillEngine;

struct GameState
{
	struct GraphicsState
	{
		VulkanDescriptorSet renderedImage;

		std::array<VulkanDescriptorSet, 6> downSampledImageDescriptorSetInput;
		std::array<VulkanDescriptorSet, 6> downSampledImageDescriptorSetOutput;

		std::array<VulkanDescriptorSet, 6> upSampledImageDescriptorSetInput;
		std::array<VulkanDescriptorSet, 6> upSampledImageDescriptorSetOutput;

		std::array<VkDescriptorSet, 6> downSampledImage_ImGui;
		std::array<VkDescriptorSet, 6> upSampledImage_ImGui;

		VulkanFramebuffer GBuffers;
	} graphicsState;
	

	struct GraphicsResources
	{
		std::map<u32, Material*> materials;
		std::vector<LightComponent*> lights;
	} graphicsResources;

	struct GameResources
	{
		std::map<u32, Entity*> entities;
	} gameResources;

	struct UIParams
	{
		u32 selectedEntityId;
	} uiParams;

	struct TodoTasks
	{
		std::queue<Entity*> meshesToAdd;
	} todoTasks;
	
	struct MaterialUpdateInfo
	{
		bool updateTexture;
		bool updateColor;
		u32 materialId;
		u32 textureIndex;
		std::string textureFilepath;
	} materialUpdateInfo;
};