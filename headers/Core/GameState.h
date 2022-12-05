#pragma once
#include "Core/Mesh.h"
#include "Core/Material.h"
#include "Core/Light.h"

#include "Core/Vulkan/VulkanDescriptorSet.h"
#include "Core/Vulkan/VulkanFramebuffer.h"

struct GameState
{
	struct GraphicsState
	{
		VulkanDescriptorSet renderedImage;

		//VkDescriptorSetLayout downSampledImageDescriptorSetLayout;
		std::array<VulkanDescriptorSet, 6> downSampledImageDescriptorSet;

		std::array<VkDescriptorSet, 6> downSampledImage_ImGui;

		VulkanFramebuffer GBuffers;
	} graphicsState;
	

	struct GraphicsResources
	{
		std::vector<Mesh*> meshes;
		std::vector<Material*> materials;
		std::vector<Light*> lights;
	} graphicsResources;

	struct PresetResources
	{
		Mesh* lightMesh;
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