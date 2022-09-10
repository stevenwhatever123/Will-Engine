#include "pch.h"

#include "Managers/SystemManager.h"

SystemManager::SystemManager() :
    vulkan(false),
    glWindow(nullptr),
    vulkanWindow(nullptr),
    keys(),
    leftMouseClicked(0),
    rightMouseClicked(0),
    modelRotation(0)
{

}

void SystemManager::init()
{
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
    vulkanWindow->init();

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

    if (!vulkan)
    {
        if (glfwWindowShouldClose(glWindow->window))
        {
            glWindow->closeWindow = true;

            glfwTerminate();

            return;
        }

        //WillEngine::printGLDebugMessage();

        glfwSwapBuffers(glWindow->window);
    }
    else
    {
        if (glfwWindowShouldClose(vulkanWindow->window))
        {
            vulkanWindow->closeWindow = true;
            vulkanWindow->cleanup();

            return;
        }

        vulkanWindow->vulkanEngine->updateSceneUniform(camera);
        vulkanWindow->update();
    }
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
            printf("Failed to read %s\n", filename);
            return;
        }
            
        std::vector<Mesh*> loadedMeshes = WillEngine::Utils::readModel(filename.c_str());

        for (Mesh* mesh : loadedMeshes)
        {
            mesh->uploadDataToPhysicalDevice(vulkanWindow->logicalDevice, vulkanWindow->physicalDevice, vulkanWindow->vulkanEngine->vmaAllocator, vulkanWindow->surface,
                vulkanWindow->graphicsQueue);

            WillEngine::VulkanUtil::createPipelineLayout(vulkanWindow->logicalDevice, mesh->pipelineLayout,
                1, &vulkanWindow->vulkanEngine->sceneDescriptorSetLayout);
            WillEngine::VulkanUtil::createPipeline(vulkanWindow->logicalDevice, mesh->pipeline, mesh->pipelineLayout,
                vulkanWindow->vulkanEngine->renderPass, mesh->vertShader, mesh->fragShader, mesh->primitive, vulkanWindow->vulkanEngine->swapchainExtent);
        }

        vulkanWindow->vulkanEngine->meshes.insert(vulkanWindow->vulkanEngine->meshes.end(), loadedMeshes.begin(), loadedMeshes.end());
    }
}

void SystemManager::updateCamera()
{
    float speed = 5.0f;

    if (keys['W']) { camera->moveForward(speed * deltaTime); }
    if (keys['S']) { camera->moveForward(-speed * deltaTime); }
    if (keys['A']) { camera->moveRight(-speed * deltaTime); }
    if (keys['D']) { camera->moveRight(speed * deltaTime); }
    if (keys['E']) { camera->moveUp(speed * deltaTime); }
    if (keys['Q']) { camera->moveUp(-speed * deltaTime); }

    camera->updateCameraMatrix();
}

void SystemManager::readFile()
{
    WillEngine::Utils::selectFile();
}

void SystemManager::useVulkan()
{
    vulkan = true;
}

void SystemManager::useOpenGL()
{
    vulkan = false;
}