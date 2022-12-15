#include "pch.h"

#include "Managers/SystemManager.h"

using namespace WillEngine;

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

    initECS();

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

    initPresets();
}

void SystemManager::initCamera()
{
    camera = new Camera(vec3(0, 0, 0), vec3(0, 0, -1));
}

void SystemManager::initLight()
{
    TransformComponent* transform = new TransformComponent();

    LightComponent* light = new LightComponent(transform->getPosition());

    // View projection matrices for 6 different side of the cube map
    // Order: +x, -x, +y, -y, +z, -z
    mat4 lightProjectionMatrix = glm::perspective(90.0f, 1.0f, 1.0f, 100.0f);

    light->matrices[0] = lightProjectionMatrix * glm::lookAt(transform->getPosition(), transform->getPosition() + vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    light->matrices[1] = lightProjectionMatrix * glm::lookAt(transform->getPosition(), transform->getPosition() + vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    light->matrices[2] = lightProjectionMatrix * glm::lookAt(transform->getPosition(), transform->getPosition() + vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
    light->matrices[3] = lightProjectionMatrix * glm::lookAt(transform->getPosition(), transform->getPosition() + vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
    light->matrices[4] = lightProjectionMatrix * glm::lookAt(transform->getPosition(), transform->getPosition() + vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f));
    light->matrices[5] = lightProjectionMatrix * glm::lookAt(transform->getPosition(), transform->getPosition() + vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));

    gameState.graphicsResources.lights.push_back(light);

    Entity* entity = new Entity();

    entity->setName("Light");
    entity->addComponent(transform);
    entity->addComponent(light);

    gameState.gameResources.entities[entity->id] = entity;
}

void SystemManager::initPresets()
{
    
}

void SystemManager::initECS()
{
    WillEngine::initComponentType();
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

    updateECS();

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

    updateGui();

    vulkanWindow->vulkanEngine->updateSceneUniform(camera);
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
            
        std::vector<MeshComponent*> loadedMeshes;
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

        for (MeshComponent* mesh : loadedMeshes)
        {
            mesh->uploadDataToPhysicalDevice(vulkanWindow->logicalDevice, vulkanWindow->physicalDevice, vulkanWindow->vulkanEngine->vmaAllocator, vulkanWindow->surface,
                vulkanWindow->graphicsQueue);
        }

        // Modify mesh's material index based on the current size of materials
        for (MeshComponent* mesh : loadedMeshes)
        {
            mesh->materialIndex = currentMaterialSize + mesh->materialIndex;
        }

        std::vector<Entity*> entities(loadedMeshes.size());

        for (u32 i = 0; i < entities.size(); i++)
        {
            entities[i] = new Entity();

            TransformComponent* transform = new TransformComponent();

            entities[i]->setName(loadedMeshes[i]->name.c_str());

            entities[i]->addComponent(transform);
            entities[i]->addComponent(loadedMeshes[i]);

            gameState.gameResources.entities[entities[i]->id] = entities[i];
        }

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

void SystemManager::updateGui()
{
    VulkanEngine* vulkanEngine = vulkanWindow->vulkanEngine;
    VulkanGui* vulkanGui = vulkanEngine->vulkanGui;

    vulkanGui->update(gameState.graphicsState.downSampledImage_ImGui[0], vulkanEngine->offscreenFramebuffer, &gameState, vulkanEngine->sceneExtent, vulkanEngine->sceneExtentChanged);
}

void SystemManager::updateECS()
{
    for(auto it = gameState.gameResources.entities.begin(); it != gameState.gameResources.entities.end(); it++)
    {
        Entity* entity = it->second;

        // Update transform
        if(entity->HasComponent<TransformComponent>())
            entity->GetComponent<TransformComponent>()->update();

        // Update light
        if (entity->HasComponent<LightComponent>())
        {
            TransformComponent* transform = entity->GetComponent<TransformComponent>();
            LightComponent* light = entity->GetComponent<LightComponent>();

            light->updateLightPosition(transform->getPosition());
            light->update();
        }
    }
}

void SystemManager::readFile()
{
    WillEngine::Utils::selectFile();
}