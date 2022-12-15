#pragma once
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

		std::array<VulkanDescriptorSet, 6> downSampledImageDescriptorSet;

		std::array<VkDescriptorSet, 6> downSampledImage_ImGui;

		VulkanFramebuffer GBuffers;
	} graphicsState;
	

	struct GraphicsResources
	{
		std::vector<Material*> materials;
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

	struct PresetResources
	{
		MeshComponent* lightMesh;
	} presetResources;
	
	struct MaterialUpdateInfo
	{
		bool updateTexture;
		bool updateColor;
		u32 materialIndex;
		u32 textureIndex;
		std::string textureFilepath;
	} materialUpdateInfo;
};