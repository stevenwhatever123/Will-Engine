#pragma once
#include "Core/UniformClass.h"

#include "Utils/Image.h"
#include "Utils/VulkanUtil.h"

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

	// Constructor
	TextureDescriptorSet();
};

class Material
{
public:

	static const u32 TEXTURE_SIZE = 5;
	static const u32 BRDF_TEXTURE_SIZE = 4;
	static const std::string TEXTURE_TYPE_NAME[TEXTURE_SIZE];

public:

	std::string name;

	// Id for this material
	const u32 id;

	// Material for BRDF
	MaterialUniform materialUniform;

	// The metallic brdf texture
	// The FIRST 4 is fixed as the BRDF materials, anything after that is textures such as normal/specular map
	// BRDF: 1. Emissive, 2. Albedo (Diffuse), 3. Metallic, 4. Roughness
	// Others: 5. Normal Map
	TextureDescriptorSet textures[TEXTURE_SIZE];

	// Descriptor Set / Uniform Buffer for vulkan
	VkDescriptorSetLayout textureDescriptorSetLayout;
	VkDescriptorSet textureDescriptorSet;

private:

	// Used for generating an id for a material
	static u32 idCounter;

public:
	Material();
	Material(const Material* material);
	~Material();

	void cleanUp(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, VkDescriptorPool& descriptorPool);

	// Setters
	void setTextureImage(u32 index, Image* image, TextureDescriptorSet* textures);

	void freeTextureImage(u32 index, TextureDescriptorSet* textures);

	// Init
	void initTexture(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool, VkQueue& graphicsQueue, 
		TextureDescriptorSet* textures, u32 index);

	void initDescriptorSet(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool,
		VkDescriptorPool& descriptorPool, VkQueue& graphicsQueue);

	// Update
	void updateDescriptorSet(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool,
		VkDescriptorPool& descriptorPool, VkQueue& graphicsQueue, u32 index);

	// Getters
	const bool hasTexture(u32 index, TextureDescriptorSet* textures);
	const char* getTexturePath(u32 index);
	PhongMaterialUniform getMaterialUniform();
};