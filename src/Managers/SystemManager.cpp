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
    Entity* entity = new Entity();

    TransformComponent* transform = new TransformComponent();

    Light* light = new Light(transform->getPosition());

    // View projection matrices for 6 different side of the cube map
    // Order: +x, -x, +y, -y, +z, -z
    mat4 lightProjectionMatrix = glm::perspective(90.0f, 1.0f, 1.0f, 100.0f);

    light->matrices[0] = lightProjectionMatrix * glm::lookAt(transform->getPosition(), transform->getPosition() + vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    light->matrices[1] = lightProjectionMatrix * glm::lookAt(transform->getPosition(), transform->getPosition() + vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    light->matrices[2] = lightProjectionMatrix * glm::lookAt(transform->getPosition(), transform->getPosition() + vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
    light->matrices[3] = lightProjectionMatrix * glm::lookAt(transform->getPosition(), transform->getPosition() + vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
    light->matrices[4] = lightProjectionMatrix * glm::lookAt(transform->getPosition(), transform->getPosition() + vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f));
    light->matrices[5] = lightProjectionMatrix * glm::lookAt(transform->getPosition(), transform->getPosition() + vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));

    LightComponent* lightComp = new LightComponent(light);

    entity->setName("Light");
    entity->addComponent(transform);
    entity->addComponent(lightComp);

    gameState.graphicsResources.lights[light->id] = light;
    gameState.gameResources.entities[entity->id] = entity;
    gameState.gameResources.rootEntities[entity->id] = entity;
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

    vulkanWindow->initVulkan(&gameState, 4);
}

void SystemManager::update()
{
    updateInputs();
    
    // Process queried tasks
    processQueriedTasks();

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
        loadModel();
    }
}

void SystemManager::updateCamera()
{
    if (!camera)
        return;

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

    //vulkanGui->update(gameState.graphicsState.upSampledImage_ImGui[0], vulkanEngine->offscreenFramebuffer, &gameState, vulkanEngine->sceneExtent, vulkanEngine->sceneExtentChanged);
    vulkanGui->update(gameState.graphicsState.renderedImage_ImGui, vulkanEngine->offscreenFramebuffer, &gameState, vulkanEngine->sceneExtent, vulkanEngine->sceneExtentChanged);
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
            LightComponent* lightComp = entity->GetComponent<LightComponent>();

            gameState.graphicsResources.lights[lightComp->lightIndex]->updateLightPosition(transform->getPosition());
            gameState.graphicsResources.lights[lightComp->lightIndex]->update();
        }
    }
}

void SystemManager::readFile()
{
    WillEngine::Utils::selectFile();
}

void SystemManager::loadModel()
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
    std::map<u32, Material*> loadedMaterials;
    Skeleton* loadedSkeleton;
    std::vector<Entity*> entities;
    std::tie(loadedMeshes, loadedMaterials, loadedSkeleton) = WillEngine::Utils::readModel(filename.c_str(), &entities);

    for (auto it = loadedMaterials.begin(); it != loadedMaterials.end(); it++)
    {
        Material* material = it->second;

        material->initDescriptorSet(vulkanWindow->logicalDevice, vulkanWindow->physicalDevice, vulkanWindow->vulkanEngine->vmaAllocator,
            vulkanWindow->vulkanEngine->commandPools[0], vulkanWindow->vulkanEngine->descriptorPool, vulkanWindow->graphicsQueue);

        // Add this material to the game state graphics resources
        gameState.graphicsResources.materials[material->id] = material;
    }

    for (Mesh* mesh : loadedMeshes)
    {
        mesh->uploadDataToPhysicalDevice(vulkanWindow->logicalDevice, vulkanWindow->physicalDevice, vulkanWindow->vulkanEngine->vmaAllocator, vulkanWindow->surface,
            vulkanWindow->graphicsQueue);

        // Add this mesh to the game state graphics resources
        gameState.graphicsResources.meshes[mesh->id] = mesh;
    }

    if (loadedSkeleton)
    {
        gameState.gameResources.skeletons[loadedSkeleton->id] = loadedSkeleton;
        vulkanWindow->vulkanEngine->skeletonToInitialise.push(loadedSkeleton);
    }

    // Root Entity is usually and always the first element
    gameState.gameResources.rootEntities[entities[0]->id] = entities[0];

    for (u32 i = 0; i < entities.size(); i++)
    {
        gameState.gameResources.entities[entities[i]->id] = entities[i];
    }

    // Add all the entities to the query to update transformation
    gameState.queryTasks.transformToUpdate.push(entities[0]);
}

void SystemManager::processQueriedTasks()
{
    processMesh();
    processTransformationCalculations();
}

void SystemManager::processMesh()
{
    while (!gameState.queryTasks.meshesToAdd.empty())
    {
        Entity* entity = gameState.queryTasks.meshesToAdd.front();

        std::string defaultPreset = "C:/Users/Steven/source/repos/Will-Engine/presets/meshes/cube.fbx";

        std::vector<Mesh*> loadedMeshes;
        std::map<u32, Material*> loadedMaterials;
        Skeleton* loadedSkeleton = nullptr;
        std::tie(loadedMeshes, loadedMaterials, loadedSkeleton) = WillEngine::Utils::readModel(defaultPreset.c_str());

        Mesh* mesh = loadedMeshes[0];
        mesh->uploadDataToPhysicalDevice(vulkanWindow->logicalDevice, vulkanWindow->physicalDevice, vulkanWindow->vulkanEngine->vmaAllocator, vulkanWindow->surface, vulkanWindow->graphicsQueue);
        // Add this mesh to the graphics resources
        gameState.graphicsResources.meshes[mesh->id] = mesh;

        // Add a this mesh as a mesh component to the entity
        MeshComponent* meshComp = new MeshComponent();
        meshComp->addMesh(mesh, loadedMaterials[mesh->materialIndex]);
        entity->addComponent(meshComp);

        // Add this material to the graphics resources
        Material* material = loadedMaterials.begin()->second;
        gameState.graphicsResources.materials[material->id] = material;

        material->initDescriptorSet(vulkanWindow->logicalDevice, vulkanWindow->physicalDevice, vulkanWindow->vulkanEngine->vmaAllocator,
            vulkanWindow->vulkanEngine->commandPools[0], vulkanWindow->vulkanEngine->descriptorPool, vulkanWindow->graphicsQueue);

        gameState.queryTasks.meshesToAdd.pop();
    }
}

void SystemManager::processTransformationCalculations()
{
    // Update Global Transformation
    while (!gameState.queryTasks.transformToUpdate.empty())
    {
        Entity* currentEntity = gameState.queryTasks.transformToUpdate.front();

        // Update Global Transformation
        TransformComponent* transformComponent = currentEntity->GetComponent<TransformComponent>();
        transformComponent->updateAllChildWorldTransformation();

        // Update Skeleton Bone Transformation
        for (auto it = gameState.gameResources.skeletons.begin(); it != gameState.gameResources.skeletons.end(); it++)
        {
            u32 skeletonId = it->first;
            Skeleton* skeleton = it->second;

            // This is a horrible way to match if the current entity (or it's child) has a skeleton or not but it will work for now
            for (auto jt = gameState.gameResources.entities.begin(); jt != gameState.gameResources.entities.end(); jt++)
            {
                u32 entityId = jt->first;
                Entity* entity = jt->second;

                // Check if they're from the same root entity
                if (entity->getRoot() != currentEntity->getRoot())
                    continue;

                if (skeleton->hasBone(entity->name))
                {
                    Entity* rootEntity = currentEntity->getRoot();
                    skeleton->updateBoneUniform(rootEntity);
                    break;
                }
            }
        }

        // Remove the current entity from the queue
        gameState.queryTasks.transformToUpdate.pop();
    }
}