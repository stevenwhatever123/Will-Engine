#include "pch.h"

#include "Managers/SystemManager.h"

SystemManager::SystemManager() :
    vulkanWindow(nullptr),
    windowWidth(0),
    windowHeight(0),
    mouseX(0),
    mouseY(0),
    currentTime(0),
    deltaTime(0),
    lastTime(0),
    camera(nullptr),
    inputManager(nullptr),
    keys(),
    leftMouseClicked(0),
    rightMouseClicked(0)
{

}

void SystemManager::init(i32 windowWidth, i32 windowHeight)
{
    this->windowWidth = windowWidth;
    this->windowHeight = windowHeight;

    initCamera();

    initVulkanWindow();
    glfwSetWindowUserPointer(vulkanWindow->window, this);

    // Init time
    currentTime = glfwGetTime();
    deltaTime = 0;
    lastTime = currentTime;
}

void SystemManager::initCamera()
{
    camera = new Camera(vec3(0, 0, 0), vec3(0, 0, -1));
}

void SystemManager::initVulkanWindow()
{
    vulkanWindow = new VulkanWindow();
    vulkanWindow->init(windowWidth, windowHeight);

    // Bind input manager to the window
    inputManager = new InputManager();
    inputManager->init(vulkanWindow->window);
}

void SystemManager::update()
{
    updateInputs();

    if (camera)
        updateCamera();

    // Update time
    currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    if (glfwWindowShouldClose(vulkanWindow->window))
    {
        vulkanWindow->closeWindow = true;
        vulkanWindow->cleanup();

        return;
    }

    vulkanWindow->vulkanEngine->updateSceneUniform(camera);
    vulkanWindow->update();
}

void SystemManager::updateInputs()
{
    /* Poll for and process events */
    glfwPollEvents();

    if (keys['L'])
    {
        bool readSuccess;
        std::string filename;

        std::tie(readSuccess, filename) = WillEngine::Utils::selectFile();

        if (!readSuccess)
        {
            printf("Failed to read %s\n", filename.c_str());
            return;
        }
            
        std::vector<Mesh*> loadedMeshes;
        std::vector<Material*> loadedMaterials;

        std::tie(loadedMeshes, loadedMaterials) = WillEngine::Utils::readModel(filename.c_str());

        u32 currentMaterialSize = materials.size();

        for (Material* material : loadedMaterials)
        {
            if (!material->hasTexture()) continue;

            material->vulkanImage = WillEngine::VulkanUtil::createImage(vulkanWindow->logicalDevice, vulkanWindow->vulkanEngine->vmaAllocator,
                material->vulkanImage.image, VK_FORMAT_R8G8B8A8_SRGB, material->width, material->height);

            WillEngine::VulkanUtil::loadTextureImage(vulkanWindow->logicalDevice, vulkanWindow->vulkanEngine->vmaAllocator,
                vulkanWindow->vulkanEngine->commandPool, vulkanWindow->graphicsQueue, material->vulkanImage, 1, material->width, material->height, material->textureImage->data);

            // Free the image from the cpu
            material->freeTextureImage();

            material->initDescriptorSet(vulkanWindow->logicalDevice, vulkanWindow->vulkanEngine->descriptorPool, 
                vulkanWindow->vulkanEngine->sampler);
        }

        for (Mesh* mesh : loadedMeshes)
        {
            mesh->uploadDataToPhysicalDevice(vulkanWindow->logicalDevice, vulkanWindow->physicalDevice, vulkanWindow->vulkanEngine->vmaAllocator, vulkanWindow->surface,
                vulkanWindow->graphicsQueue);

            VkDescriptorSetLayout layouts[] = { vulkanWindow->vulkanEngine->sceneDescriptorSetLayout ,
                loadedMaterials[mesh->materialIndex]->descriptorSetLayout };

            u32 descriptorSetLayoutSize = sizeof(layouts) / sizeof(layouts[0]);

            WillEngine::VulkanUtil::createPipelineLayout(vulkanWindow->logicalDevice, mesh->pipelineLayout,
                descriptorSetLayoutSize, layouts);
            WillEngine::VulkanUtil::createPipeline(vulkanWindow->logicalDevice, mesh->pipeline, mesh->pipelineLayout,
                vulkanWindow->vulkanEngine->renderPass, mesh->vertShader, mesh->fragShader, mesh->primitive, vulkanWindow->vulkanEngine->swapchainExtent);
        
            mesh->dataUploaded();
        }

        // Modify mesh's material index based on the current size of materials
        for (Mesh* mesh : loadedMeshes)
        {
            mesh->materialIndex = currentMaterialSize + mesh->materialIndex;
        }

        vulkanWindow->vulkanEngine->meshes.insert(vulkanWindow->vulkanEngine->meshes.end(), loadedMeshes.begin(), loadedMeshes.end());
        vulkanWindow->vulkanEngine->materials.insert(vulkanWindow->vulkanEngine->materials.end(), loadedMaterials.begin(), loadedMaterials.end());
    }
}

void SystemManager::updateCamera()
{
    f32 speed = 5.0f;

    if (keys['W']) { camera->moveForward(speed * (f32) deltaTime); }
    if (keys['S']) { camera->moveForward(-speed * (f32) deltaTime); }
    if (keys['A']) { camera->moveRight(-speed * (f32) deltaTime); }
    if (keys['D']) { camera->moveRight(speed * (f32) deltaTime); }
    if (keys['E']) { camera->moveUp(speed * (f32) deltaTime); }
    if (keys['Q']) { camera->moveUp(-speed * (f32) deltaTime); }

    camera->updateCameraMatrix();
}

void SystemManager::readFile()
{
    WillEngine::Utils::selectFile();
}