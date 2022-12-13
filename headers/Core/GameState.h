#pragma once
#include "Core/MeshComponent.h"
#include "Core/Material.h"
#include "Core/Light.h"

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
		std::vector<MeshComponent*> meshes;
		std::vector<Material*> materials;
		std::vector<Light*> lights;
	} graphicsResources;

	struct GameResources
	{
		std::vector<Entity*> entities;
	} gameResources;

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