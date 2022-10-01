#include "pch.h"
#include "Managers/InputManager.h"

#include "Managers/SystemManager.h"

InputManager::InputManager()
{

}

InputManager::~InputManager()
{

}

void InputManager::init(GLFWwindow* systemWindow)
{
    // Keyboard handler
    auto key_callback = [](GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods)
    {
        glfwGetWindowUserPointer(window);

        SystemManager* system = (SystemManager*)glfwGetWindowUserPointer(window);

        if (key >= GLFW_KEY_SPACE && key < GLFW_KEY_ESCAPE)
        {
            if (action == GLFW_PRESS) system->keys[(char)key] = 1;
            if (action == GLFW_RELEASE) system->keys[(char)key] = 0;
        }
    };
    glfwSetKeyCallback(systemWindow, key_callback);


    // Mouse key handler
    auto mouse_key_callback = [](GLFWwindow* window, i32 key, i32 action, i32 mods)
    {
        glfwGetWindowUserPointer(window);

        SystemManager* system = (SystemManager*)glfwGetWindowUserPointer(window);

        if (key == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (action == GLFW_PRESS) system->leftMouseClicked = 1;
            else if (action == GLFW_RELEASE) system->leftMouseClicked = 0;
        }
        if (key == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if (action == GLFW_PRESS) system->rightMouseClicked = 1;
            else if (action == GLFW_RELEASE) system->rightMouseClicked = 0;
        }
    };
    glfwSetMouseButtonCallback(systemWindow, mouse_key_callback);


    // Mouse movement handler
    auto mouse_pos_callback = [](GLFWwindow* window, double xpos, double ypos)
    {
        glfwGetWindowUserPointer(window);

        SystemManager* system = (SystemManager*)glfwGetWindowUserPointer(window);

        if (system->leftMouseClicked)
        {
            double lastMouseX = system->mouseX;
            double lastMouseY = system->mouseY;

            double rotateX = xpos - lastMouseX;
            double rotateY = ypos - lastMouseY;
        }

        system->mouseX = xpos;
        system->mouseY = ypos;
    };
    glfwSetCursorPosCallback(systemWindow, mouse_pos_callback);
}