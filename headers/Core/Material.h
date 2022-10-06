#pragma once
#include "Utils/Image.h"

#include "Utils/VulkanUtil.h"

struct MaterialUniform
{
	// std140 layout
	vec4 color;
	u32 has_texture;			// Boolean 4 byte layout
};

class Material
{
public:

	std::string name;

	vec4 color;

	// Texture?
	bool has_texture;
	std::string texture_path;

	bool useTexture;

	// Texture details
	i32 width, height, numChannels;
	Image* textureImage;
	u32 mipLevels;

	// Texture image in vulkan
	VulkanAllocatedImage vulkanImage;
	VkImageView imageView;
	VkSampler textureSampler;

	// Descriptor Set / Uniform Buffer for vulkan
	VkDescriptorSetLayout textureDescriptorSetLayout;
	VkDescriptorSet textureDescriptorSet;

	// Texture Descriptor Set for imgui UI
	VkDescriptorSet imguiTextureDescriptorSet;

	VkDescriptorSetLayout materialDescriptorSetLayout;
	VkDescriptorSet materialDescriptorSet;

public:
	Material();
	~Material();

	void cleanUp(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, VkDescriptorPool& descriptorPool);

	// Setters
	void setTextureImage(Image* image);

	void freeTextureImage();

	// Init
	void initDescriptorSet(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool,
		VkDescriptorPool& descriptorPool, VkQueue& graphicsQueue);

	void updateTextureDesciptorSet(VkDevice& logicalDevice, VkDescriptorSet& descriptorSet);

	// Getters
	const bool hasTexture() { return has_texture; }
	const char* getTexturePath();
	MaterialUniform getMaterialUniform();
};