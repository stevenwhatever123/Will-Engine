#include "pch.h"
#include "Core/Material.h"

u32 Material::idCounter = 0;

TextureDescriptorSet::TextureDescriptorSet():
	has_texture(false),
	texture_path(""),
	useTexture(false),
	width(1),
	height(1),
	numChannels(0),
	textureImage(new Image),
	mipLevels(0),
	vulkanImage({ VK_NULL_HANDLE,VK_NULL_HANDLE }),
	imageView(VK_NULL_HANDLE),
	textureSampler(VK_NULL_HANDLE),
	imguiTextureDescriptorSet(VK_NULL_HANDLE)
{

}

// Initialise the static const texture type for Material class
const std::string Material::TEXTURE_TYPE_NAME[TEXTURE_SIZE] = { "Emissive", "Albedo", "Metallic", "Roughness", "Normal"};

Material::Material() :
	materialUniform({}),
	name(""),
	id(++idCounter),
	textures(),
	textureDescriptorSetLayout(VK_NULL_HANDLE),
	textureDescriptorSet(VK_NULL_HANDLE)
{

}

Material::Material(const Material* material) :
	materialUniform({}),
	name(material->name.c_str()),
	id(material->id),
	textures(),
	textureDescriptorSetLayout(material->textureDescriptorSetLayout),
	textureDescriptorSet(material->textureDescriptorSet)
{
	std::copy(std::begin(material->textures), std::end(material->textures), std::begin(textures));
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

void Material::setTextureImage(u32 index, Image* image, TextureDescriptorSet* textures)
{
	textures[index].textureImage = image;
}

void Material::freeTextureImage(u32 index, TextureDescriptorSet* textures)
{
	textures[index].textureImage->freeImage();
	delete textures[index].textureImage;
}

void Material::initTexture(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool, VkQueue& graphicsQueue, 
	TextureDescriptorSet* textures, u32 index)
{
	// Calculate mip levels
	textures[index].mipLevels = WillEngine::VulkanUtil::calculateMiplevels(textures[index].width, textures[index].height);

	// Create vulkan image
	textures[index].vulkanImage = WillEngine::VulkanUtil::createImage(logicalDevice, vmaAllocator,
		VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, textures[index].width, textures[index].height, textures[index].mipLevels);

	// Load image to physical device with mipmapping
	WillEngine::VulkanUtil::loadTextureImageWithMipmap(logicalDevice, vmaAllocator,
		commandPool, graphicsQueue, textures[index].vulkanImage, textures[index].mipLevels, textures[index].width, textures[index].height, textures[index].textureImage->data);

	// Free the image from memory
	freeTextureImage(index, textures);

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
	for (u32 i = 0; i < TEXTURE_SIZE; i++)
	{
		initTexture(logicalDevice, physicalDevice, vmaAllocator, commandPool, graphicsQueue, textures, i);
	}

	// Initialise Descriptor set layout first
	// Binding set to 1 with n number of descriptor sets
	WillEngine::VulkanUtil::createDescriptorSetLayout(logicalDevice, textureDescriptorSetLayout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT, 1, TEXTURE_SIZE);

	// Allocate memory for descriptor set
	WillEngine::VulkanUtil::allocDescriptorSet(logicalDevice, descriptorPool, textureDescriptorSetLayout, textureDescriptorSet);

	// Write Descriptor Set
	std::vector<VkSampler> textureSamplers(TEXTURE_SIZE);
	std::vector<VkImageView> imageViews(TEXTURE_SIZE);
	for (u32 i = 0; i < TEXTURE_SIZE; i++)
	{
		textureSamplers[i] = textures[i].textureSampler;
		imageViews[i] = textures[i].imageView;
	}

	WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, textureDescriptorSet, textureSamplers.data(), imageViews.data(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, TEXTURE_SIZE);
}

void Material::updateDescriptorSet(VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VmaAllocator& vmaAllocator, VkCommandPool& commandPool,
	VkDescriptorPool& descriptorPool, VkQueue& graphicsQueue, u32 index)
{
	// Free previous memory
	vmaDestroyImage(vmaAllocator, textures[index].vulkanImage.image, textures[index].vulkanImage.allocation);

	vkDestroyImageView(logicalDevice, textures[index].imageView, nullptr);

	vkDestroySampler(logicalDevice, textures[index].textureSampler, nullptr);

	// Update the image and imageview associated to it
	initTexture(logicalDevice, physicalDevice, vmaAllocator, commandPool, graphicsQueue, textures, index);

	// Write Descriptor Set
	std::vector<VkSampler> textureSamplers(TEXTURE_SIZE);
	std::vector<VkImageView> imageViews(TEXTURE_SIZE);
	for (u32 i = 0; i < TEXTURE_SIZE; i++)
	{
		textureSamplers[i] = textures[i].textureSampler;
		imageViews[i] = textures[i].imageView;
	}

	WillEngine::VulkanUtil::writeDescriptorSetImage(logicalDevice, textureDescriptorSet, textureSamplers.data(), imageViews.data(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, TEXTURE_SIZE);
}

const bool Material::hasTexture(u32 index, TextureDescriptorSet* textures)
{
	return textures[index].has_texture;
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