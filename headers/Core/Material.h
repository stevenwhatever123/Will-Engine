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

// Metallic-Roughness Material model 
struct BRDFMetallic
{
	vec4 albedo;
	f32 metallic;
	f32 roughness;
};

// Specular-Glossiness Material model
struct BRDFSpecular
{
	// Leave this for now, I'm going to work on this later
	vec4 albedo;
	vec4 specular;
	f32 glossiness;
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

	// Constructor
	TextureDescriptorSet();
};

class Material
{
public:

	std::string name;

	// Material for Phong shading
	PhongMaterialUniform phongMaterialUniform;

	// Material for BRDF
	BRDFMetallic brdfMaterialUniform;

	// The default texture would always be in the order of:
	// Phong: 1. Emissive, 2. Ambient, 3. Diffuse, 4. Specular
	TextureDescriptorSet textures[4];

	// The metallic brdf texture
	// BRDF: 1. Emissive, 2. Ambient, 3. Albedo (Diffuse), 4. Metallic, 5. Roughness
	TextureDescriptorSet brdfTextures[5];

	// Descriptor Set / Uniform Buffer for vulkan
	VkDescriptorSetLayout textureDescriptorSetLayout;
	VkDescriptorSet textureDescriptorSet;

public:
	Material();
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
	void initBrdfDescriptorSet(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool,
		VkDescriptorPool& descriptorPool, VkQueue& graphicsQueue);

	// Update
	void updateDescriptorSet(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool,
		VkDescriptorPool& descriptorPool, VkQueue& graphicsQueue, u32 index);
	void updateBrdfDescriptorSet(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool,
		VkDescriptorPool& descriptorPool, VkQueue& graphicsQueue, u32 index);

	// Getters
	const bool hasTexture(u32 index, TextureDescriptorSet* textures);
	const char* getTexturePath(u32 index);
	PhongMaterialUniform getMaterialUniform();
};