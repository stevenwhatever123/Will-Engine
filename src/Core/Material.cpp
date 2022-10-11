#include "pch.h"
#include "Core/Material.h"

TextureDescriptorSet::TextureDescriptorSet():
	has_texture(false),
	texture_path(""),
	width(1),
	height(1),
	numChannels(0),
	textureImage(nullptr),
	mipLevels(0),
	vulkanImage({ VK_NULL_HANDLE,VK_NULL_HANDLE }),
	imageView(VK_NULL_HANDLE),
	textureSampler(VK_NULL_HANDLE),
	imguiTextureDescriptorSet(VK_NULL_HANDLE)
{

}

Material::Material() :
	phongMaterialUniform({}),
	name(""),
	textures(),
	textureDescriptorSetLayout(VK_NULL_HANDLE),
	textureDescriptorSet(VK_NULL_HANDLE)
{

}

Material::~Material()
{

}

void Material::cleanUp(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, VkDescriptorPool& descriptorPool)
{
	for (auto& texture : textures)
	{
		vmaDestroyImage(vmaAllocator, texture.vulkanImage.image, texture.vulkanImage.allocation);

		vkDestroyImageView(logicalDevice, texture.imageView, nullptr);

		vkDestroySampler(logicalDevice, texture.textureSampler, nullptr);
	}

	vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &textureDescriptorSet);

	vkDestroyDescriptorSetLayout(logicalDevice, textureDescriptorSetLayout, nullptr);
}

void Material::setTextureImage(u32 index, Image* image)
{
	this->textures[index].textureImage = image;
}

void Material::freeTextureImage(u32 index)
{
	textures[index].textureImage->freeImage(hasTexture(index));
	delete this->textures[index].textureImage;
}

void Material::initTexture(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool, VkQueue& graphicsQueue, u32 index)
{
	// Calculate mip levels
	textures[index].mipLevels = WillEngine::VulkanUtil::calculateMiplevels(textures[index].width, textures[index].height);

	// Create vulkan image
	textures[index].vulkanImage = WillEngine::VulkanUtil::createImage(logicalDevice, vmaAllocator,
		textures[index].vulkanImage.image, VK_FORMAT_R8G8B8A8_SRGB, textures[index].width, textures[index].height, textures[index].mipLevels);

	// Load image to physical device with mipmapping
	WillEngine::VulkanUtil::loadTextureImageWithMipmap(logicalDevice, vmaAllocator,
		commandPool, graphicsQueue, textures[index].vulkanImage, textures[index].mipLevels, textures[index].width, textures[index].height, textures[index].textureImage->data);

	// Free the image from memory
	freeTextureImage(index);

	// Create texture sampler
	WillEngine::VulkanUtil::createTextureSampler(logicalDevice, physicalDevice, textures[index].textureSampler, textures[index].mipLevels);

	// Create a sampler and image view for the texture image
	WillEngine::VulkanUtil::createImageView(logicalDevice, textures[index].vulkanImage.image, textures[index].imageView, textures[index].mipLevels, VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_ASPECT_COLOR_BIT);

	// Descriptor Set for imgui texture
	textures[index].imguiTextureDescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(textures[index].textureSampler, textures[index].imageView,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Material::initDescriptorSet(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool,
	VkDescriptorPool& descriptorPool, VkQueue& graphicsQueue)
{
	// Initiase Texture
	for (u32 i = 0; i < 4; i++)
	{
		initTexture(logicalDevice, physicalDevice, vmaAllocator, commandPool, graphicsQueue, i);
	}

	// Initialise Descriptor set layout first
	// Binding set to 1 with 4 descriptor sets
	WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, textureDescriptorSetLayout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
		VK_SHADER_STAGE_FRAGMENT_BIT, 1, 4);
	
	// Allocate memory for descriptor set
	WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, textureDescriptorSetLayout, textureDescriptorSet);

	// Write Descriptor Set
	std::vector<VkSampler> textureSamplers(4);
	std::vector<VkImageView> imageViews(4);
	for (u32 i = 0; i < 4; i++)
	{
		textureSamplers[i] = textures[i].textureSampler;
		imageViews[i] = textures[i].imageView;
	}

	WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, textureDescriptorSet, textureSamplers, imageViews,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 4);
}

void Material::updateDescriptorSet(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool,
	VkDescriptorPool& descriptorPool, VkQueue& graphicsQueue, u32 index)
{
	// Free previous memory
	vmaDestroyImage(vmaAllocator, textures[index].vulkanImage.image, textures[index].vulkanImage.allocation);

	vkDestroyImageView(logicalDevice, textures[index].imageView, nullptr);

	vkDestroySampler(logicalDevice, textures[index].textureSampler, nullptr);

	// Update the image and imageview associated to it
	initTexture(logicalDevice, physicalDevice, vmaAllocator, commandPool, graphicsQueue, index);

	// Write Descriptor Set
	std::vector<VkSampler> textureSamplers(4);
	std::vector<VkImageView> imageViews(4);
	for (u32 i = 0; i < 4; i++)
	{
		textureSamplers[i] = textures[i].textureSampler;
		imageViews[i] = textures[i].imageView;
	}

	WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, textureDescriptorSet, textureSamplers, imageViews,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 4);
}

const char* Material::getTexturePath(u32 index)
{
	return textures[index].texture_path.c_str();
}

PhongMaterialUniform Material::getMaterialUniform()
{
	PhongMaterialUniform material_uniform{};
	//material_uniform.has_texture = (u32)this->has_texture;
	//material_uniform.color = this->color;

	return material_uniform;
}