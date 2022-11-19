#pragma once
#include "Core/Mesh.h"
#include "Core/Material.h"
#include "Core/Light.h"

#include "Core/Vulkan/VulkanFramebuffer.h"

struct GameState
{
	struct GraphicsState
	{
		VkDescriptorSet renderedImage_ImGui;

		VkDescriptorSetLayout renderedImageLayout;
		VkDescriptorSet renderedImage;

		VkDescriptorSet computedImage_ImGui;

		VkDescriptorSetLayout computedImageLayout;
		VkDescriptorSet computedImage;

		VulkanFramebuffer GBuffers;
	} graphicsState;
	

	struct GraphicsResources
	{
		std::vector<Mesh*> meshes;
		std::vector<Material*> materials;
		std::vector<Light*> lights;
	} graphicsResources;
	
	struct MaterialUpdateInfo
	{
		bool updateTexture;
		bool updateColor;
		u32 materialIndex;
		u32 textureIndex;
		std::string textureFilepath;
	} materialUpdateInfo;
};