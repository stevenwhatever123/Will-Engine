#pragma once

typedef u32 PipelineId;

struct VulkanAllocatedMemory
{
	VkBuffer buffer;
	VmaAllocation allocation;
};

struct VulkanAllocatedImage
{
	VkImage image;
	VkImageView imageView;
	VmaAllocation allocation;
};

enum VulkanAllocatedImageType : u8
{

};

struct VulkanPostProcessingImages
{
	std::vector<VulkanAllocatedImage> allocatedImages;
};

struct VulkanDescriptorSet
{
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout layout;
	VulkanAllocatedMemory buffer;
};

enum class VulkanDescriptorSetType : u8
{
	DepthSkeletal,
	Skeletal,
	Scene, 
	Light,
	LightMatrix,
	Camera,
	Texture,
	Attachment,
	ShadowMap
};

enum class VulkanPipelineType : u8
{
	Depth,
	Geometry,
	Shading,
	Shadow,
	DepthSkeletal,
	Skeletal,

	FilterBright,
	Downscale,
	Upscale,
	BlendColor
};

enum class VulkanCommandBufferType : u8
{
	Depth,
	Geometry,
	Shading,
	Shadow,
	DepthSkeletal,
	Skeletal,

	FilterBright,
	Downscale,
	Upscale,
	BlendColor,

	Upload,
	UniformUpdate,
	Present
};

struct VulkanPipeline
{
	VkPipeline pipeline;
	VkPipelineLayout layout;
};

enum class VulkanShaderType : u8
{
	Vert,
	Frag,
	Geom,
	Comp
};

struct VulkanShaderModule
{
	std::unordered_map<VulkanShaderType, VkShaderModule> shaders;
};

enum class VulkanSamplerType : u8
{
	Default,
	Attachment,
	Shadow
};

enum class VulkanRenderPassType : u8
{
	Depth,
	Geometry,
	Shadow,
	Shading,
	Present
};

enum class VulkanSemaphoreType : u8
{
	ImageAvailable,
	UniformUpdate,
	RenderFinished,

	PreDepthFinished,
	ShadowFinished,
	GeometryFinished,

	DownscaleFinished,
	UpscaleFinished,

	ColorBlendFinished,

	ReadyToPresent,

	End
};

enum VulkanFramebufferType : u8
{
	Depth,
	Geometry,
	Shading,
	ShadowMap
};

enum VulkanPostProcessingType : u8
{
	Downscale,
	Upscale
};

struct VulkanFramebufferAttachment
{
	VulkanAllocatedImage vulkanImage;
	VkImageView imageView;
	VkFormat format;

	// For ImGui UI
	VkDescriptorSet imguiTextureDescriptorSet;
};

struct VulkanFramebuffer
{
	static const u32 ATTACHMENT_SIZE = 4;

	VkFramebuffer framebuffer;
	std::vector<VulkanFramebufferAttachment> attachments;

	void cleanUp(VkDevice& logicalDevice, VmaAllocator& vmaAllocator);
};