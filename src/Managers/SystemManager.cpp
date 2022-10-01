#include "pch.h"

#include "Managers/SystemManager.h"

SystemManager::SystemManager() :
    vulkanWindow(nullptr),
    windowWidth(0),
    windowHeight(0),
    lastMouseX(0),
    lastMouseY(0),
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
    vulkanWindow->createWindow(windowWidth, windowHeight);

    // Bind input manager to the window
    inputManager = new InputManager();
    inputManager->init(vulkanWindow->window);

    vulkanWindow->initVulkan();
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
    lastMouseX = mouseX;
    lastMouseY = mouseY;

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
    if (keys['W']) { camera->moveForward(movementSpeed * (f32) deltaTime); }
    if (keys['S']) { camera->moveForward(-movementSpeed * (f32) deltaTime); }
    if (keys['A']) { camera->moveRight(-movementSpeed * (f32) deltaTime); }
    if (keys['D']) { camera->moveRight(movementSpeed * (f32) deltaTime); }
    if (keys['E']) { camera->moveUp(movementSpeed * (f32) deltaTime); }
    if (keys['Q']) { camera->moveUp(-movementSpeed * (f32) deltaTime); }

    if (rightMouseClicked)
    {
        f32 xDistance = mouseX - lastMouseX;
        f32 yDistance = mouseY - lastMouseY;

        f32 mouseMovementY = -yDistance * mouseSpeed * (f32) deltaTime;
        f32 mouseMoveMentX = -xDistance * mouseSpeed * (f32) deltaTime;

        camera->rotate(mouseMovementY, mouseMoveMentX, 0);
    }

    camera->updateCameraMatrix();
}

void SystemManager::readFile()
{
    WillEngine::Utils::selectFile();
}