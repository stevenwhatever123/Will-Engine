#include "pch.h"
#include "Core/Material.h"

Material::Material() :
	phongMaterialUniform({}),
	name(""),
	has_texture(false),
	texture_path(""),
	useTexture(false),
	width(1),
	height(1),
	numChannels(4),
	textureImage(new Image()),
	mipLevels(1),
	vulkanImage({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	imageView(VK_NULL_HANDLE),
	textureSampler(VK_NULL_HANDLE),
	textureDescriptorSetLayout(VK_NULL_HANDLE),
	textureDescriptorSet(VK_NULL_HANDLE),
	imguiTextureDescriptorSet(VK_NULL_HANDLE),
	materialDescriptorSetLayout(VK_NULL_HANDLE),
	materialDescriptorSet(VK_NULL_HANDLE)
{

}

Material::~Material()
{

}

void Material::cleanUp(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, VkDescriptorPool& descriptorPool)
{
	vmaDestroyImage(vmaAllocator, vulkanImage.image, vulkanImage.allocation);

	vkDestroyImageView(logicalDevice, imageView, nullptr);

	vkDestroySampler(logicalDevice, textureSampler, nullptr);

	vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &textureDescriptorSet);

	vkDestroyDescriptorSetLayout(logicalDevice, textureDescriptorSetLayout, nullptr);
}

void Material::setTextureImage(Image* image)
{
	this->textureImage = image;
}

void Material::freeTextureImage()
{
	textureImage->freeImage(hasTexture());
	delete this->textureImage;
}

void Material::initDescriptorSet(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool,
	VkDescriptorPool& descriptorPool, VkQueue& graphicsQueue)
{
	// Calculate mip levels
	mipLevels = WillEngine::VulkanUtil::calculateMiplevels(width, height);

	// Create vulkan image
	vulkanImage = WillEngine::VulkanUtil::createImage(logicalDevice, vmaAllocator,
		vulkanImage.image, VK_FORMAT_R8G8B8A8_SRGB, width, height, mipLevels);

	// Load image to physical device with mipmapping
	WillEngine::VulkanUtil::loadTextureImageWithMipmap(logicalDevice, vmaAllocator,
		commandPool, graphicsQueue, vulkanImage, mipLevels, width, height, textureImage->data);

	// Free the image from memory
	freeTextureImage();

	// Create texture sampler
	WillEngine::VulkanUtil::createTextureSampler(logicalDevice, physicalDevice, textureSampler, mipLevels);

	// Initialise Descriptor set layout first
	// Binding set to 1
	WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, textureDescriptorSetLayout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
		VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);

	// Create a sampler and image view for the texture image
	WillEngine::VulkanUtil::createImageView(logicalDevice, vulkanImage.image, imageView, mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	
	// Allocate descriptor set
	WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, textureDescriptorSetLayout, textureDescriptorSet);

	// Descriptor Set for imgui texture
	imguiTextureDescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(textureSampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// Write Descriptor Set
	WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, textureDescriptorSet, textureSampler, imageView,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
}

const char* Material::getTexturePath()
{
	return texture_path.c_str();
}

PhongMaterialUniform Material::getMaterialUniform()
{
	PhongMaterialUniform material_uniform{};
	//material_uniform.has_texture = (u32)this->has_texture;
	//material_uniform.color = this->color;

	return material_uniform;
}