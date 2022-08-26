#pragma once
class GLWindow
{
private:

public:

	GLFWwindow* window;

	bool closeWindow;

	i32 windowWidth, windowHeight;

	i32 imguiWidth, imguiHeight;

public:

	GLWindow();
	~GLWindow();

	void init();
	void update();

	// Return
	bool shouldCloseWindow() const { return closeWindow; };
};

