#include "pch.h"
#include "Core/OpenGL/GLWindow.h"

GLWindow::GLWindow()
{

}

GLWindow::~GLWindow()
{

}

void GLWindow::init()
{
    closeWindow = false;

    /* Initialize the library */
    if (!glfwInit())
        closeWindow = true;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1600, 900, "Will Engine - OpenGL", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        closeWindow = true;
        return;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, this);
}

void GLWindow::update()
{
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
}