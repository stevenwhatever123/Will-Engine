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
}

void VulkanGui::cleanUp(VkDevice& logicalDevice)
{
	vkDestroyDescriptorPool(logicalDevice, imguiDescriptorPool, nullptr);

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void VulkanGui::update(std::vector<Mesh*>& meshes, std::vector<Material*>& materials, std::vector<Light*>& lights, bool& updateTexture, bool& updateColor, 
	u32& materialIndex, u32& textureIndex, std::string& textureFilepath)
{
	ImGuiIO& io = ImGui::GetIO();

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if(show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Begin("Meshes Viewer");

	for (u32 i = 0; i < meshes.size(); i++)
	{
		ImGui::PushID(i);

		if (ImGui::TreeNode(meshes[i]->name.c_str()))
		{

			ImGui::DragFloat3("Position", &meshes[i]->transformPosition.x, 0.1f);

			ImGui::Text("Material: %s", materials[meshes[i]->materialIndex]->name.c_str());

			ImGui::Image((ImTextureID) materials[meshes[i]->materialIndex]->textures[2].imguiTextureDescriptorSet, ImVec2(200, 200));

			ImGui::TreePop();
		}

		ImGui::PopID();
	}
	ImGui::End();



	ImGui::Begin("Material Viewer");

	std::string phongMaterials[4] = { "Emissive", "Ambient", "Diffuse", "Specular" };
	std::string brdfMaterials[5] = { "Emissive", "Ambient", "Albedo", "Metallic", "Roughness" };

	for (u32 i = 0; i < materials.size(); i++)
	{
		if (ImGui::TreeNode(materials[i]->name.c_str()))
		{
			ImGui::PushID(i);

			if (ImGui::BeginTable("Phong material", 1, ImGuiTableFlags_BordersV))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Phong material properties");

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				vec4 emissiveTemp = materials[i]->phongMaterialUniform.emissiveColor;
				ImGui::DragFloat3("Emissive", &materials[i]->phongMaterialUniform.emissiveColor.x, 0.01f, 0, 1);
				vec4 emissiveDiff = glm::epsilonNotEqual(emissiveTemp, materials[i]->phongMaterialUniform.emissiveColor, 0.0001f);
				if (emissiveDiff.x || emissiveDiff.y || emissiveDiff.z)
				{
					updateColor = true;
					materialIndex = i;
					textureIndex = 0;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				vec4 ambientTemp = materials[i]->phongMaterialUniform.ambientColor;
				ImGui::DragFloat3("Ambient", &materials[i]->phongMaterialUniform.ambientColor.x, 0.01f, 0, 1);
				vec4 ambientDiff = glm::epsilonNotEqual(ambientTemp, materials[i]->phongMaterialUniform.ambientColor, 0.0001f);
				if (ambientDiff.x || ambientDiff.y || ambientDiff.z)
				{
					updateColor = true;
					materialIndex = i;
					textureIndex = 1;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				vec4 diffuseTemp = materials[i]->phongMaterialUniform.diffuseColor;
				ImGui::DragFloat3("Diffuse", &materials[i]->phongMaterialUniform.diffuseColor.x, 0.01f, 0, 1);
				vec4 diffuseDiff = glm::epsilonNotEqual(diffuseTemp, materials[i]->phongMaterialUniform.diffuseColor, 0.0001f);
				if (diffuseDiff.x || diffuseDiff.y || diffuseDiff.z)
				{
					updateColor = true;
					materialIndex = i;
					textureIndex = 2;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				vec4 specularTemp = materials[i]->phongMaterialUniform.specularColor;
				ImGui::DragFloat3("Specular", &materials[i]->phongMaterialUniform.specularColor.x, 0.01f, 0, 1);
				vec4 specularDiff = glm::epsilonNotEqual(specularTemp, materials[i]->phongMaterialUniform.specularColor, 0.0001f);
				if (specularDiff.x || specularDiff.y || specularDiff.z)
				{
					updateColor = true;
					materialIndex = i;
					textureIndex = 3;
				}

				ImGui::EndTable();
			}

			// BRDF materials
			if (ImGui::BeginTable("BRDF material", 1, ImGuiTableFlags_BordersV))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("BRDF material properties");

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				vec4 emissiveTemp = materials[i]->phongMaterialUniform.emissiveColor;
				ImGui::DragFloat3("Emissive", &materials[i]->phongMaterialUniform.emissiveColor.x, 0.01f, 0, 1);
				vec4 emissiveDiff = glm::epsilonNotEqual(emissiveTemp, materials[i]->phongMaterialUniform.emissiveColor, 0.0001f);
				if (emissiveDiff.x || emissiveDiff.y || emissiveDiff.z)
				{
					updateColor = true;
					materialIndex = i;
					textureIndex = 0;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				vec4 ambientTemp = materials[i]->phongMaterialUniform.ambientColor;
				ImGui::DragFloat3("Ambient", &materials[i]->phongMaterialUniform.ambientColor.x, 0.01f, 0, 1);
				vec4 ambientDiff = glm::epsilonNotEqual(ambientTemp, materials[i]->phongMaterialUniform.ambientColor, 0.0001f);
				if (ambientDiff.x || ambientDiff.y || ambientDiff.z)
				{
					updateColor = true;
					materialIndex = i;
					textureIndex = 1;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				vec4 albedoTemp = materials[i]->brdfMaterialUniform.albedo;
				ImGui::DragFloat3("Albedo", &materials[i]->brdfMaterialUniform.albedo.x, 0.01f, 0, 1);
				vec4 albedoDiff = glm::epsilonNotEqual(albedoTemp, materials[i]->brdfMaterialUniform.albedo, 0.0001f);
				if (albedoDiff.x || albedoDiff.y || albedoDiff.z)
				{
					updateColor = true;
					materialIndex = i;
					textureIndex = 2;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				f32 metallicTemp = materials[i]->brdfMaterialUniform.metallic;
				ImGui::DragFloat("Metallic", &materials[i]->brdfMaterialUniform.metallic, 0.01f, 0, 1);
				f32 metallicDiff = glm::epsilonNotEqual(metallicTemp, materials[i]->brdfMaterialUniform.metallic, 0.0001f);
				if (metallicDiff)
				{
					updateColor = true;
					materialIndex = i;
					textureIndex = 3;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				f32 roughnessTemp = materials[i]->brdfMaterialUniform.roughness;
				ImGui::DragFloat("Roughness", &materials[i]->brdfMaterialUniform.roughness, 0.01f, 0, 1);
				f32 roughnessDiff = glm::epsilonNotEqual(roughnessTemp, materials[i]->brdfMaterialUniform.roughness, 0.0001f);
				if (roughnessDiff)
				{
					updateColor = true;
					materialIndex = i;
					textureIndex = 3;
				}

				ImGui::EndTable();
			}

			// BRDF materials
			for (u32 j = 0; j < 5; j++)
			{
				ImGui::PushID(j);

				ImGui::Text(brdfMaterials[j].c_str());

				bool lastUseTexture = materials[i]->brdfTextures[j].useTexture;
				ImGui::Checkbox("Use Texture", &materials[i]->brdfTextures[j].useTexture);

				bool checkBoxChanged = (lastUseTexture == true && materials[i]->brdfTextures[j].useTexture == false)
					|| (lastUseTexture == false && materials[i]->brdfTextures[j].useTexture == true);

				if (checkBoxChanged)
				{
					if (materials[i]->brdfTextures[j].useTexture)
					{
						updateTexture = true;
						materialIndex = i;
						textureIndex = j;
					}
					else
					{
						updateColor = true;
						materialIndex = i;
						textureIndex = j;
					}
				}

				if (materials[i]->hasTexture(j, materials[i]->brdfTextures) || materials[i]->brdfTextures[j].useTexture)
				{
					ImGui::Text("Texture path: %s", materials[i]->brdfTextures[j].texture_path.c_str());
					if (ImGui::ImageButton((ImTextureID)materials[i]->brdfTextures[j].imguiTextureDescriptorSet, ImVec2(150, 150)))
					{
						bool readSuccess;
						std::string filename;

						std::tie(readSuccess, filename) = WillEngine::Utils::selectFile();

						if (!readSuccess)
						{
							printf("Failed to read %s\n", filename.c_str());

							updateTexture = false;
							materialIndex = 0;
							textureIndex = 0;
							textureFilepath = "";
						}
						else
						{
							updateTexture = true;
							materialIndex = i;
							textureIndex = j;
							textureFilepath = filename;
						}
					}
				}

				ImGui::PopID();
			}

			// Phong materials
			//for (u32 j = 0; j < 4; j++)
			//{
			//	ImGui::PushID(j);

			//	ImGui::Text(phongMaterials[j].c_str());

			//	bool lastUseTexture = materials[i]->textures[j].useTexture;
			//	ImGui::Checkbox("Use Texture", &materials[i]->textures[j].useTexture);

			//	bool checkBoxChanged = (lastUseTexture == true && materials[i]->textures[j].useTexture == false)
			//		|| (lastUseTexture == false && materials[i]->textures[j].useTexture == true);

			//	if (checkBoxChanged)
			//	{
			//		if (materials[i]->textures[j].useTexture)
			//		{
			//			updateTexture = true;
			//			materialIndex = i;
			//			textureIndex = j;
			//		}
			//		else
			//		{
			//			updateColor = true;
			//			materialIndex = i;
			//			textureIndex = j;
			//		}
			//	}

			//	if (materials[i]->hasTexture(j, materials[i]->textures) || materials[i]->textures[j].useTexture)
			//	{
			//		if (ImGui::ImageButton((ImTextureID)materials[i]->textures[j].imguiTextureDescriptorSet, ImVec2(150, 150)))
			//		{
			//			bool readSuccess;
			//			std::string filename;

			//			std::tie(readSuccess, filename) = WillEngine::Utils::selectFile();

			//			if (!readSuccess)
			//			{
			//				printf("Failed to read %s\n", filename.c_str());

			//				updateTexture = false;
			//				materialIndex = 0;
			//				textureIndex = 0;
			//				textureFilepath = "";
			//			}
			//			else
			//			{
			//				updateTexture = true;
			//				materialIndex = i;
			//				textureIndex = j;
			//				textureFilepath = filename;
			//			}
			//		}
			//	}

			//	ImGui::PopID();
			//}

			ImGui::PopID();

			ImGui::TreePop();
		}
	}

	ImGui::End();



	if (lights.size() > 0)
	{
		ImGui::Begin("Light Control");

		for (u32 i = 0; i < lights.size(); i++)
		{
			ImGui::PushID(i);

			ImGui::DragFloat3("", &lights[i]->position.x, 0.1f);

			ImGui::SameLine();

			ImGui::Text("Light %u", i);

			ImGui::PopID();
		}

		ImGui::End();
	}
}

void VulkanGui::renderUI(VkCommandBuffer& commandBuffer, VkExtent2D extent)
{
	ImGui::Render();
	// Record ImGui rendering command
	ImDrawData* draw_data = ImGui::GetDrawData();
	const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
	if (!is_minimized)
	{
		ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
	}
}