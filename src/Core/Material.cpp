#include "pch.h"
#include "Core/Material.h"

Material::Material() :
	color(0),
	name(""),
	has_texture(false),
	texture_path(""),
	width(1),
	height(1),
	numChannels(4),
	textureImage(new Image()),
	vulkanImage({ VK_NULL_HANDLE, VK_NULL_HANDLE }),
	imageView(VK_NULL_HANDLE),
	textureDescriptorSetLayout(VK_NULL_HANDLE),
	textureDescriptorSet(VK_NULL_HANDLE),
	materialDescriptorSetLayout(VK_NULL_HANDLE),
	materialDescriptorSet(VK_NULL_HANDLE)
{

}

Material::~Material()
{

}

void Material::cleanUp(VkDevice& logicalDevice, VmaAllocator& vmaAllocator, VkDescriptorPool& descriptorPool)
{
	//if (!hasTexture())
	//	return;

	vmaDestroyImage(vmaAllocator, vulkanImage.image, vulkanImage.allocation);

	vkDestroyImageView(logicalDevice, imageView, nullptr);

	vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &textureDescriptorSet);

	vkDestroyDescriptorSetLayout(logicalDevice, textureDescriptorSetLayout, nullptr);
}

void Material::setTextureImage(Image* image)
{
	this->textureImage = image;
}

void Material::freeTextureImage()
{
	textureImage->freeImage();
	delete this->textureImage;
}

void Material::initDescriptorSet(VkDevice& logicalDevice, VkDescriptorPool& descriptorPool, VkSampler& sampler)
{
	// Initialise Descriptor set layout first
	WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, textureDescriptorSetLayout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

	// Create a sampler and image view for the texture image
	WillEngine::VulkanUtil::createImageView(logicalDevice, vulkanImage.image, imageView, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	
	// Allocate descriptor set
	WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, textureDescriptorSetLayout, textureDescriptorSet);

	// Update Descriptor Set
	updateTextureDesciptorSet(logicalDevice, textureDescriptorSet, sampler);
}

void Material::updateTextureDesciptorSet(VkDevice& logicalDevice, VkDescriptorSet& descriptorSet, VkSampler& sampler)
{
	VkDescriptorImageInfo imageInfo{};
	imageInfo.sampler = sampler;
	imageInfo.imageView = imageView;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet writeSet{};
	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.dstSet = descriptorSet;
	writeSet.dstBinding = 0;
	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeSet.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(logicalDevice, 1, &writeSet, 0, nullptr);
}

const char* Material::getTexturePath()
{
	return texture_path.c_str();
}

MaterialUniform Material::getMaterialUniform()
{
	MaterialUniform material_uniform{};
	material_uniform.has_texture = (u32)this->has_texture;
	material_uniform.color = this->color;

	return material_uniform;
}