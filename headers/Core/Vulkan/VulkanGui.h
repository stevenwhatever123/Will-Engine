#pragma once
#include "Core/MeshComponent.h"
#include "Core/Material.h"
#include "Core/LightComponent.h"

#include "Core/GameState.h"

#include "Core/Vulkan/VulkanDefines.h"

// Panels
#include "Core/EngineGui/ScenePanel.h"
#include "Core/EngineGui/EntitiesPanel.h"
#include "Core/EngineGui/MaterialPanel.h"
#include "Core/EngineGui/DebuggingPanel.h"
#include "Core/EngineGui/InspectorPanel.h"

class VulkanGui
{
private:

	VkDescriptorPool imguiDescriptorPool = VK_NULL_HANDLE;
	VkCommandBuffer imguiCommandBuffer = VK_NULL_HANDLE;

public:

	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	bool firstLoop = true;

public:

	VulkanGui();
	~VulkanGui();

	void init(GLFWwindow* window, VkInstance& instance, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface,
		u32 queueFamily, VkCommandPool& commandPool, VkDescriptorPool& descriptorPool, u32 imageCount, VkRenderPass& renderPass, VkExtent2D extent);
	void cleanUp(VkDevice& logicalDevice);

	void setLayout();

	void updateMenuBar();

	void update(VkDescriptorSet& shadedImage, VulkanFramebuffer& attachments, GameState* gamestate, VkExtent2D& sceneExtent, bool& sceneExtentChanged);

	void renderUI(VkCommandBuffer& commandBuffer, VkExtent2D extent);

	// Getters
	VkDescriptorPool& getDescriptorPool();
};