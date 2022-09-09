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

        if (readSuccess)
        {
            std::vector<Mesh*> loadedMeshes = WillEngine::Utils::readModel(filename.c_str());

            for (Mesh* mesh : loadedMeshes)
            {
                mesh->sendDataToGPU(vulkanWindow->logicalDevice, vulkanWindow->physicalDevice, vulkanWindow->vulkanEngine->vmaAllocator, vulkanWindow->surface,
                    vulkanWindow->graphicsQueue);

                mesh->generatePipelineLayout(vulkanWindow->logicalDevice, vulkanWindow->vulkanEngine->sceneDescriptorSetLayout);
                mesh->generatePipeline(vulkanWindow->logicalDevice, vulkanWindow->vulkanEngine->renderPass, vulkanWindow->vulkanEngine->swapchainExtent);
            }

            vulkanWindow->vulkanEngine->meshes.insert(vulkanWindow->vulkanEngine->meshes.end(), loadedMeshes.begin(), loadedMeshes.end());
        }
    }
}

void SystemManager::updateCamera()
{
    if (keys['W']) { camera->moveForward(0.01f); }
    if (keys['S']) { camera->moveForward(-0.01f); }
    if (keys['A']) { camera->moveRight(-0.01f); }
    if (keys['D']) { camera->moveRight(0.01f); }
    if (keys['E']) { camera->moveUp(0.01f); }
    if (keys['Q']) { camera->moveUp(-0.01f); }

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