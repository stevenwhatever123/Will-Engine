#pragma once
#include "Managers/FileManager.h"
#include "Managers/InputManager.h"
#include "Core/ImGuiUI.h"
#include "Core/Renderer.h"
#include "Core/Camera.h"
#include "Utils/Logging.h"

class SystemManager
{
public:
	GLFWwindow* window;
	i32 windowWidth, windowHeight;

	i32 imguiWidth, imguiHeight;

	f64 mouseX, mouseY;			// Current mouse xy position

	f64 currentTime;
	f64 deltaTime;
	f64 lastTime;

	u64 frameCount = 0;
	f64 timePasses = 0;
	f64 fpsAverage = 0;

	// Cores
	Renderer* renderer;
	
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
	void initGLWindow();
	void initShaders();
	void initCamera();
	void initRenderer();
	void initImgui();


	// Updates
	void update();
	void updateGLWindow();
	void updateInputs();
	void updateCamera();
	void updateRenderer();
	void updateImgui();

	// Utils
	void readFile();

	// Keyboard / Mouse
	u32 keys[256];
	bool leftMouseClicked;
	bool rightMouseClicked;

	// Return
	bool closeWindow;
	bool shouldCloseWindow() const { return closeWindow; };
};