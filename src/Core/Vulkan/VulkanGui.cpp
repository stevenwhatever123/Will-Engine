#include "pch.h"

#include "Core/Vulkan/VulkanGui.h"

#include "Utils/VulkanUtil.h"

VulkanGui::VulkanGui()
{

}

VulkanGui::~VulkanGui()
{

}

void VulkanGui::init(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, u32 queueFamily, 
	VkCommandPool& commandPool, VkDescriptorPool& descriptorPool, u32 imageCount, VkRenderPass& renderPass, VkExtent2D extent)
{
	// Create a descriptor pool for ImGui
	// the size of the pool is very oversize, but it's copied from imgui demo itself.
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	poolInfo.pPoolSizes = pool_sizes;

	if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &imguiDescriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor Pool");

	// Initialise ImGui backend
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.WantCaptureMouse = true;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(window, true);

	// Retrive the graphics queue
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	vkGetDeviceQueue(logicalDevice, queueFamily, 0, &graphicsQueue);

	// Load vulkan function pointer for ImGui
	static VkInstance globalInstance = instance;
	ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void*) { return vkGetInstanceProcAddr(globalInstance, function_name);});

	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = instance;
	initInfo.Device = logicalDevice;
	initInfo.PhysicalDevice = physicalDevice;
	initInfo.QueueFamily = queueFamily;
	initInfo.Queue = graphicsQueue;
	initInfo.DescriptorPool = imguiDescriptorPool;
	initInfo.Subpass = 0;
	initInfo.MinImageCount = imageCount;
	initInfo.ImageCount = imageCount;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = nullptr;
	ImGui_ImplVulkan_Init(&initInfo, renderPass);

	// Upload ImGui fonts
	imguiCommandBuffer = WillEngine::VulkanUtil::createCommandBuffer(logicalDevice, commandPool);
	
	vkResetCommandPool(logicalDevice, commandPool, 0);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(imguiCommandBuffer, &beginInfo);

	ImGui_ImplVulkan_CreateFontsTexture(imguiCommandBuffer);

	vkEndCommandBuffer(imguiCommandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &imguiCommandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

	vkDeviceWaitIdle(logicalDevice);

	ImGui_ImplVulkan_DestroyFontUploadObjects();

	// Vulkan window for Imgui
}

void VulkanGui::cleanUp(VkDevice& logicalDevice)
{
	vkDestroyDescriptorPool(logicalDevice, imguiDescriptorPool, nullptr);

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void VulkanGui::update()
{

}

void VulkanGui::renderUI(VkCommandBuffer& commandBuffer, VkExtent2D extent)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Render();

	// Record ImGui rendering command
	ImDrawData* draw_data = ImGui::GetDrawData();
	const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
	if (!is_minimized)
	{
		ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
	}
}