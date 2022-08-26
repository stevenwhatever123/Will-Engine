#pragma once
class GLRenderer
{
private:

	mat4 projectionMatrix;

public:

	i32 width, height;
	vec4 clearColor;

public:
	GLRenderer(int width, int height);
	GLRenderer();
	~GLRenderer();

	// Function commands
	void init();
	void draw();

	// Setters
	void setFramebufferSize(const int width, const int height);
	void setClearColor(const vec4 &color);

};