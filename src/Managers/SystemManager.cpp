#include "pch.h"

#include "Managers/SystemManager.h"

SystemManager::SystemManager() :
    simulate(false),
    keys(),
    left_mouse_clicked(0),
    right_mouse_clicked(0),
    mouseX(0),
    mouseY(0),
    modelRotation(0)
{
    window = nullptr;
    closeWindow = false;
}

void SystemManager::init()
{
    init_window();
    init_shaders();
    init_renderer();
    init_imgui();

    // Init time
    currentTime = glfwGetTime();
    deltaTime = 0;
    lastTime = currentTime;
}

void SystemManager::init_window()
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

    // Keyboard handler
    auto key_callback = [](GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods)
    {
        glfwGetWindowUserPointer(window);

        SystemManager* system = (SystemManager*) glfwGetWindowUserPointer(window);
        if (key >= GLFW_KEY_SPACE && key < GLFW_KEY_ESCAPE)
        {
            if (action == GLFW_PRESS) system->keys[(char) key] = 1;
            if (action == GLFW_RELEASE) system->keys[(char) key] = 0;
        }

        if (key == GLFW_KEY_ESCAPE) system->closeWindow = true;
    };
    glfwSetKeyCallback(window, key_callback);

    // Mouse key handler
    auto mouse_key_callback = [](GLFWwindow* window, i32 key, i32 action, i32 mods)
    {
        glfwGetWindowUserPointer(window);

        SystemManager* system = (SystemManager*)glfwGetWindowUserPointer(window);
        if (key == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (action == GLFW_PRESS) system->left_mouse_clicked = 1;
            else if (action == GLFW_RELEASE) system->left_mouse_clicked = 0;
        }
        if (key == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if (action == GLFW_PRESS) system->right_mouse_clicked = 1;
            else if (action == GLFW_RELEASE) system->right_mouse_clicked = 0;
        }
    };
    glfwSetMouseButtonCallback(window, mouse_key_callback);
    
    // Mouse movement handler
    auto mouse_pos_callback = [](GLFWwindow* window, double xpos, double ypos)
    {
        glfwGetWindowUserPointer(window);

        SystemManager* system = (SystemManager*)glfwGetWindowUserPointer(window);

        if (system->left_mouse_clicked)
        {
            double lastMouseX = system->mouseX;
            double lastMouseY = system->mouseY;

            double rotateX = xpos - lastMouseX;
            double rotateY = ypos - lastMouseY;

            system->modelRotation.x += rotateX;
            system->modelRotation.y += rotateY;
        }

        system->mouseX = xpos;
        system->mouseY = ypos;
    };
    glfwSetCursorPosCallback(window, mouse_pos_callback);
}

void SystemManager::init_shaders()
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


void SystemManager::init_renderer()
{
    //Camera* camera = new Camera(vec3(0, 0, 0), vec3(0, 0, -1));

    renderer = new Renderer(windowWidth, windowHeight);

    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    renderer->setFramebufferSize(windowWidth, windowHeight);

    //renderer->addShaderPrograms(shaderPrograms);
}

void SystemManager::init_imgui()
{
    WillEngine::UI::init_imgui(window);
}

void SystemManager::update()
{
    update_inputs();

    if (glfwWindowShouldClose(window))
    {
        closeWindow = true;

        glfwTerminate();
    }

    // Update time
    currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    // Debug Log
    //auto MessageCallback = [](GLenum source,
    //    GLenum type,
    //    GLuint id,
    //    GLenum severity,
    //    GLsizei length,
    //    const GLchar* message,
    //    const void* userParam)
    //{
    //    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
    //        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
    //        type, severity, message);
    //};
    //
    //glEnable(GL_DEBUG_OUTPUT);
    //glDebugMessageCallback(MessageCallback, 0);

    if (simulate)
    {
        frameCount++;
        timePasses += deltaTime;
        
        printf("Frame count: %d\n", frameCount);
        printf("Delta time average: %lf\n", timePasses / frameCount);
        printf("Fps average: %lf\n", (f64) frameCount / timePasses);
    }

    update_camera();

    update_renderer();

    update_imgui();

    glfwSwapBuffers(window);
}

void SystemManager::update_inputs()
{
    /* Poll for and process events */
    glfwPollEvents();

    //if (keys['W']) { renderer->getCamera()->moveForward(0.01f); }
    //if (keys['S']) { renderer->getCamera()->moveForward(-0.01f); }
    //if (keys['A']) { renderer->getCamera()->moveRight(-0.01f); }
    //if (keys['D']) { renderer->getCamera()->moveRight(0.01f); }
    //if (keys['E']) { renderer->getCamera()->moveUp(0.01f); }
    //if (keys['Q']) { renderer->getCamera()->moveUp(-0.01f); }

    //if (keys[' ']) { simulate = !simulate; }
}

void SystemManager::update_camera()
{
    //renderer->getCamera()->updateTransformation();
}

void SystemManager::update_renderer()
{
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    if (windowWidth != renderer->width
        || windowHeight != renderer->height)
    {
        renderer->setFramebufferSize(windowWidth, windowHeight);
    }

    renderer->draw();
}

void SystemManager::update_imgui()
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