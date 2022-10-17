#pragma once
#include "Managers/FileManager.h"
#include "Managers/InputManager.h"
#include "Core/Vulkan/VulkanWindow.h"
#include "Core/Camera.h"
#include "Core/Mesh.h"
#include "Core/Material.h"
#include "Core/Light.h"
#include "Utils/Logging.h"
#include "Utils/ModelImporter.h"

class SystemManager
{
private:

	bool renderWithBRDF = true;

	std::vector<Mesh*> meshes;
	std::vector<Material*> materials;

	std::vector<Light*> lights;

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
	void initLight();

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