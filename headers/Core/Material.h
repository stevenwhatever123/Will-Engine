#pragma once
#include "Utils/Image.h"

#include "Utils/VulkanUtil.h"

struct PhongMaterialUniform
{
	// std140 layout
	vec4 emissiveColor;
	vec4 ambientColor;
	vec4 diffuseColor;
	vec4 specularColor;
};

struct BRDFMaterialUniform
{

};

class Material
{
public:

	std::string name;

	PhongMaterialUniform phongMaterialUniform;

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

	void writeTextureDesciptorSet(VkDevice& logicalDevice, VkDescriptorSet& descriptorSet);

	// Getters
	const bool hasTexture() { return has_texture; }
	const char* getTexturePath();
	PhongMaterialUniform getMaterialUniform();
};