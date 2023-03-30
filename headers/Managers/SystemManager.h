#pragma once
#include "Managers/FileManager.h"
#include "Managers/InputManager.h"
#include "Managers/AnimationManager.h"

#include "Core/Vulkan/VulkanWindow.h"
#include "Core/Camera.h"
#include "Core/Mesh.h"
#include "Core/MeshComponent.h"
#include "Core/Material.h"
#include "Core/ECS/Entity.h"
#include "Core/Light.h"
#include "Core/LightComponent.h"

#include "Utils/Logging.h"
#include "Utils/ModelImporter.h"

#include "Core/ECS/TransformComponent.h"
#include "Core/ECS/AnimationComponent.h"

#include "Core/GameState.h"

#include "Core/EngineGui/ScenePanel.h"

class SystemManager
{
public:

	// Number of threads this program can use
	static const u32 MAX_THREAD = 4;

private:

	bool renderWithBRDF = true;

	GameState gameState;

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
	AnimationManager* animationManager;

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
	void initThreads();
	void initCamera();
	void initLight();
	void initPresets();
	void initECS();
	void initAnimation();

	// Vulkan init
	void initVulkanWindow();

	// Updates
	void update();

	void updateInputs();
	void updateCamera();
	void updateGui();
	void updateECS();
	void updateAnimation(float dt);

	// Utils
	void readFile();

	// Return
	bool shouldCloseWindow() { return vulkanWindow->closeWindow; }

	// Command calls
	void loadModel();

	// Process Queried Tasks
	void processQueriedTasks();

	void processMesh();
	void processTransformationCalculations();
};