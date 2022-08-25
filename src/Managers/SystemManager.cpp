#include "pch.h"

#include "Managers/SystemManager.h"

SystemManager::SystemManager() :
    keys(),
    leftMouseClicked(0),
    rightMouseClicked(0),
    mouseX(0),
    mouseY(0),
    modelRotation(0)
{
    window = nullptr;
    closeWindow = false;
}

void SystemManager::init()
{
    initGLWindow();
    initShaders();
    initCamera();
    initRenderer();
    initImgui();

    // Init time
    currentTime = glfwGetTime();
    deltaTime = 0;
    lastTime = currentTime;
}

void SystemManager::initGLWindow()
{
    closeWindow = false;

    /* Initialize the library */
    if (!glfwInit())
        closeWindow = true;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1600, 900, "Hair Simulation", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        closeWindow = true;
        return;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, this);

    gladLoadGL();

    WillEngine::printSystemInfo();

    inputManager = new InputManager();
    inputManager->init(this->window);
}

void SystemManager::initShaders()
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

void SystemManager::initCamera()
{
    camera = new Camera(vec3(0, 0, 0), vec3(0, 0, -1));
}

void SystemManager::initRenderer()
{
    renderer = new Renderer(windowWidth, windowHeight);

    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    if(windowWidth != renderer->width || windowHeight != renderer->height)
        renderer->setFramebufferSize(windowWidth, windowHeight);

    //renderer->addShaderPrograms(shaderPrograms);
}

void SystemManager::initImgui()
{
    WillEngine::UI::init_imgui(window);
}

void SystemManager::update()
{
    updateInputs();

    if (glfwWindowShouldClose(window))
    {
        closeWindow = true;

        glfwTerminate();
    }

    // Update time
    currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    updateGLWindow();

    if(camera)
        updateCamera();

    updateRenderer();

    updateImgui();

    //WillEngine::printGLDebugMessage();

    glfwSwapBuffers(window);
}

void SystemManager::updateGLWindow()
{
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    if (windowWidth != renderer->width
        || windowHeight != renderer->height)
    {
        renderer->setFramebufferSize(windowWidth, windowHeight);
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

void SystemManager::updateRenderer()
{
    renderer->draw();
}

void SystemManager::updateImgui()
{
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    if (windowWidth < 1 || windowHeight < 1)
        return;

    WillEngine::UI::update_imgui();
}

void SystemManager::readFile()
{
    WillEngine::Utils::selectFile();
}