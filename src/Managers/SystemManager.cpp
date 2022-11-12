#include "pch.h"

#include "Managers/SystemManager.h"

SystemManager::SystemManager() :
    gameState(),
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
    initLight();

    initVulkanWindow();
    glfwSetWindowUserPointer(vulkanWindow->window, this);

    // Add camera to vulkan engine
    vulkanWindow->vulkanEngine->camera = camera;

    // Init time
    currentTime = glfwGetTime();
    deltaTime = 0;
    lastTime = currentTime;
}

void SystemManager::initCamera()
{
    camera = new Camera(vec3(0, 0, 0), vec3(0, 0, -1));
}

void SystemManager::initLight()
{
    Light* light = new Light();

    // View projection matrices for 6 different side of the cube map
    // Order: +x, -x, +y, -y, +z, -z
    mat4 lightProjectionMatrix = glm::perspective(90.0f, 1.0f, 1.0f, 100.0f);

    light->matrices[0] = lightProjectionMatrix * glm::lookAt(light->position, light->position + vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    light->matrices[1] = lightProjectionMatrix * glm::lookAt(light->position, light->position + vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    light->matrices[2] = lightProjectionMatrix * glm::lookAt(light->position, light->position + vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
    light->matrices[3] = lightProjectionMatrix * glm::lookAt(light->position, light->position + vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
    light->matrices[4] = lightProjectionMatrix * glm::lookAt(light->position, light->position + vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f));
    light->matrices[5] = lightProjectionMatrix * glm::lookAt(light->position, light->position + vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));

    gameState.graphicsResources.lights.push_back(light);
}

void SystemManager::initVulkanWindow()
{
    vulkanWindow = new VulkanWindow();
    vulkanWindow->createWindow(windowWidth, windowHeight);

    // Bind input manager to the window
    inputManager = new InputManager();
    inputManager->init(vulkanWindow->window);

    vulkanWindow->initVulkan(&gameState);
}

void SystemManager::update()
{
    updateInputs();

    if (camera)
        updateCamera();

    updateLight();

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

    //updateGui();

    vulkanWindow->vulkanEngine->updateSceneUniform(camera);
    vulkanWindow->vulkanEngine->updateLightUniform(camera);
    vulkanWindow->update(renderWithBRDF);
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

        u32 currentMaterialSize = gameState.graphicsResources.materials.size();

        for (Material* material : loadedMaterials)
        {
            if (renderWithBRDF)
            {
                material->initBrdfDescriptorSet(vulkanWindow->logicalDevice, vulkanWindow->physicalDevice, vulkanWindow->vulkanEngine->vmaAllocator,
                    vulkanWindow->vulkanEngine->commandPool, vulkanWindow->vulkanEngine->descriptorPool, vulkanWindow->graphicsQueue);
            }
            else
            {
                material->initDescriptorSet(vulkanWindow->logicalDevice, vulkanWindow->physicalDevice, vulkanWindow->vulkanEngine->vmaAllocator,
                    vulkanWindow->vulkanEngine->commandPool, vulkanWindow->vulkanEngine->descriptorPool, vulkanWindow->graphicsQueue);
            }
        }

        for (Mesh* mesh : loadedMeshes)
        {
            mesh->uploadDataToPhysicalDevice(vulkanWindow->logicalDevice, vulkanWindow->physicalDevice, vulkanWindow->vulkanEngine->vmaAllocator, vulkanWindow->surface,
                vulkanWindow->graphicsQueue);
        }

        // Modify mesh's material index based on the current size of materials
        for (Mesh* mesh : loadedMeshes)
        {
            mesh->materialIndex = currentMaterialSize + mesh->materialIndex;
        }

        gameState.graphicsResources.meshes.insert(gameState.graphicsResources.meshes.end(), loadedMeshes.begin(), loadedMeshes.end());
        gameState.graphicsResources.materials.insert(gameState.graphicsResources.materials.end(), loadedMaterials.begin(), loadedMaterials.end());
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

void SystemManager::updateLight()
{
    for (u32 i = 0; i < gameState.graphicsResources.lights.size(); i++)
    {
        gameState.graphicsResources.lights[i]->update();
    }
}

void SystemManager::updateGui()
{
    WillEngine::EngineGui::ScenePanel::update(gameState.graphicsState.renderedImage);
}

void SystemManager::readFile()
{
    WillEngine::Utils::selectFile();
}