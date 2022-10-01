#pragma once
#include "Managers/FileManager.h"
#include "Managers/InputManager.h"
#include "Core/OpenGL/GLWindow.h"
#include "Core/OpenGL/GLRenderer.h"
#include "Core/OpenGL/GLImGuiUI.h"
#include "Core/Vulkan/VulkanWindow.h"
#include "Core/Camera.h"
#include "Core/Mesh.h"
#include "Utils/Logging.h"
#include "Utils/ModelImporter.h"

class SystemManager
{
private:

	std::vector<Mesh*> meshes;
	std::vector<Material*> materials;

public:

	VulkanWindow *vulkanWindow;
	i32 windowWidth, windowHeight;
	//i32 imguiWidth, imguiHeight;

	f64 lastMouseX, lastMouseY;
	f64 mouseX, mouseY;			// Current mouse xy position

	f64 currentTime;
	f64 deltaTime;
	f64 lastTime;

	// Cores
	Camera* camera;

	// Managers
	InputManager* inputManager;

	// Keyboard / Mouse
	u32 keys[256];
	bool leftMouseClicked;
	bool rightMouseClicked;

	f32 movementSpeed = 100.0f;
	f32 mouseSpeed = 100.0f;

public:

	SystemManager();
	~SystemManager();

	// Initialise
	void init(i32 windowWidth, i32 windowHeight);
	void initCamera();

	// Vulkan init
	void initVulkanWindow();

	// Updates
	void update();
	void updateInputs();
	void updateCamera();

	// Utils
	void readFile();

	// Return
	bool shouldCloseWindow() { return vulkanWindow->closeWindow; }

	// Command calls
};