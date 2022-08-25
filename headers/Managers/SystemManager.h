#pragma once
#include "Managers/FileManager.h"
#include "ImGuiUI.h"
#include "Renderer.h"
#include "Utils/Logging.h"

class SystemManager
{
private:
	GLFWwindow* window;
	int windowWidth, windowHeight;

	int imguiWidth, imguiHeight;

	double mouseX, mouseY;			// Current mouse xy position

	f64 currentTime;
	f64 deltaTime;
	f64 lastTime;

	u64 frameCount = 0;
	f64 timePasses = 0;
	f64 fpsAverage = 0;

	// Cores
	Renderer* renderer;

public:
	bool simulate;

	// For the model matrix
	vec3 modelRotation;

public:

	SystemManager();
	~SystemManager();

	// Initialise
	void init();
	void init_window();
	void init_shaders();
	void init_renderer();
	void init_imgui();


	// Updates
	void update();
	void update_inputs();
	void update_camera();
	void update_renderer();
	void update_imgui();

	// Utils
	void readFile();

	// Keyboard / Mouse
	unsigned int keys[256];
	
	bool left_mouse_clicked;
	bool right_mouse_clicked;

	// Return
	bool closeWindow;
	bool shouldCloseWindow() const { return closeWindow; };
};