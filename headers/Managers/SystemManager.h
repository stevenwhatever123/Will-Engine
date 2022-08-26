#pragma once
#include "Managers/FileManager.h"
#include "Managers/InputManager.h"
#include "Core/OpenGL/GLWindow.h"
#include "Core/OpenGL/GLRenderer.h"
#include "Core/OpenGL/GLImGuiUI.h"
#include "Core/Vulkan/VulkanWindow.h"
#include "Core/Camera.h"
#include "Utils/Logging.h"

class SystemManager
{
private:

	bool vulkan;

public:

	GLWindow* glWindow;

	VulkanWindow *vulkanWindow;
	//i32 windowWidth, windowHeight;

	//i32 imguiWidth, imguiHeight;

	f64 mouseX, mouseY;			// Current mouse xy position

	f64 currentTime;
	f64 deltaTime;
	f64 lastTime;

	// Cores
	GLRenderer* glRenderer;
	
	Camera* camera;

	// For the model matrix
	vec3 modelRotation;

	// Managers
	InputManager* inputManager;

public:

	SystemManager();
	~SystemManager();

	// Initialise
	void init();
	void initCamera();

	// OpenGL init
	void initGLWindow();
	void initGLShaders();
	void initGLRenderer();
	void initGLImgui();

	// Vulkan init
	void initVulkanWindow();



	// Updates
	void update();
	void updateInputs();
	void updateCamera();

	// Opengl update
	void updateGLWindow();
	void updateGLRenderer();
	void updateGLImgui();

	// Utils
	void readFile();

	// Keyboard / Mouse
	u32 keys[256];
	bool leftMouseClicked;
	bool rightMouseClicked;

	// Return
	bool shouldCloseWindow() const { return glWindow? glWindow->closeWindow : vulkanWindow->closeWindow; };

	// Command calls
	void useVulkan();
	void useOpenGL();
};