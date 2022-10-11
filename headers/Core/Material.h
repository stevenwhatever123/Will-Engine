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

struct TextureDescriptorSet
{
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

	// Texture Descriptor Set for imgui UI
	VkDescriptorSet imguiTextureDescriptorSet;
};

class Material
{
public:

	std::string name;

	PhongMaterialUniform phongMaterialUniform;

	// The texture would always be in the layout of:
	// 1. Emissive, 2. Ambient, 3. Diffuse, 4. Specular
	std::vector<TextureDescriptorSet> textures;

	// Descriptor Set / Uniform Buffer for vulkan
	VkDescriptorSetLayout textureDescriptorSetLayout;
	VkDescriptorSet textureDescriptorSet;

public:
	Material();
	~Material();

	void cleanUp(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, VkDescriptorPool& descriptorPool);

	// Setters
	void setTextureImage(u32 index, Image* image);

	void freeTextureImage(u32 index);

	// Init
	void initDescriptorSet(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool,
		VkDescriptorPool& descriptorPool, VkQueue& graphicsQueue);

	void initTexture(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool, VkQueue& graphicsQueue, u32 index);

	void updateDescriptorSet(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool,
		VkDescriptorPool& descriptorPool, VkQueue& graphicsQueue, u32 index);

	// Getters
	const bool hasTexture(u32 index) { return textures[index].has_texture; }
	const char* getTexturePath(u32 index);
	PhongMaterialUniform getMaterialUniform();
};