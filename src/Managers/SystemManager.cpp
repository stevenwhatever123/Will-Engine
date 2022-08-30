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
    if (!vulkan)
    {
        initGLWindow();
        initGLShaders();
        initCamera();
        initGLRenderer();
        initGLImgui();
    }
    else
    {
        initVulkanWindow();
        initCamera();
    }

    // Init time
    currentTime = glfwGetTime();
    deltaTime = 0;
    lastTime = currentTime;
}

void SystemManager::initCamera()
{
    camera = new Camera(vec3(0, 0, 0), vec3(0, 0, -1));
}

void SystemManager::initGLWindow()
{
    glWindow = new GLWindow();
    glWindow->init();

    WillEngine::printGLSystemInfo();

    // Bind input manager to the window
    inputManager = new InputManager();
    inputManager->init(glWindow->window);
}

void SystemManager::initGLShaders()
{
    //// The mesh shader      - 0
    //GLShader* meshShaderProgram = new GLShader();
    //meshShaderProgram->loadShader("shader/headVertexShader.glsl", "shader/headFragmentShader.glsl");

    //shaderPrograms.emplace_back(meshShaderProgram);

    //currentProgram = shaderPrograms[0];

    //// The hair root shader     - 1
    //GLShader* hairRootShaderProgram = new GLShader();
    //hairRootShaderProgram->loadShader("shader/hairRootVertexShader.glsl", "shader/hairRootFragmentShader.glsl");

    //shaderPrograms.emplace_back(hairRootShaderProgram);
}

void SystemManager::initGLRenderer()
{
    glRenderer = new GLRenderer(glWindow->windowWidth, glWindow->windowHeight);

    if(glWindow->windowWidth != glRenderer->width || glWindow->windowHeight != glRenderer->height)
        glRenderer->setFramebufferSize(glWindow->windowWidth, glWindow->windowHeight);

    //renderer->addShaderPrograms(shaderPrograms);
}

void SystemManager::initGLImgui()
{
    WillEngine::UI::init_glImgui(glWindow->window);
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

        updateGLWindow();

        updateGLRenderer();

        updateGLImgui();

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

        vulkanWindow->update();
    }
}

void SystemManager::updateInputs()
{
    /* Poll for and process events */
    glfwPollEvents();
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

void SystemManager::updateGLWindow()
{
    glWindow->update();
}

void SystemManager::updateGLRenderer()
{
    // Resizing if window size changed
    if (glWindow->windowWidth != glRenderer->width
        || glWindow->windowHeight != glRenderer->height)
    {
        glRenderer->setFramebufferSize(glWindow->windowWidth, glWindow->windowHeight);
    }

    glRenderer->draw();
}

void SystemManager::updateGLImgui()
{
    if (glWindow->windowWidth < 1 || glWindow->windowHeight < 1)
        return;

    WillEngine::UI::update_glImgui();
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