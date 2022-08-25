#pragma once
class Renderer
{
private:

public:

	int width, height;
	vec4 clearColor;

public:
	Renderer(int width, int height);
	Renderer();
	~Renderer();

	// Function commands
	void init();
	void draw();

	// Setters
	void setFramebufferSize(const int width, const int height);
	void setClearColor(const vec4 &color);

};