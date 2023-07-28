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
	ShadowMap,

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

class VulkanFramebufferAttachment
{
public:

	VulkanAllocatedImage vulkanImage;
	VkImageView imageView;
	VkFormat format;

	// For ImGui UI
	VkDescriptorSet imguiTextureDescriptorSet;

public:

	VulkanFramebufferAttachment();
	~VulkanFramebufferAttachment();
};

class VulkanFramebuffer
{
public:

	static const u32 attachmentSize = 4;

public:

	VkFramebuffer framebuffer;
	VulkanFramebufferAttachment GBuffer0;
	VulkanFramebufferAttachment GBuffer1;
	VulkanFramebufferAttachment GBuffer2;
	VulkanFramebufferAttachment GBuffer3;

public:

	VulkanFramebuffer();
	VulkanFramebuffer(VkFramebuffer framebuffer);
	~VulkanFramebuffer();

	void cleanUp(VkDevice& logicalDevice, VmaAllocator& vmaAllocator);
};