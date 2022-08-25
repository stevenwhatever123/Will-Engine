#include "pch.h"
#include "Renderer.h"

Renderer::Renderer(int width, int height):
	width(width),
	height(height),
	clearColor(0)
{
	init();
}

void Renderer::init()
{
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glEnable(GL_PROGRAM_POINT_SIZE);

	glDepthFunc(GL_LEQUAL);

	setClearColor(vec4(0.4f, 0.4f, 0.4f, 1));
}

void Renderer::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setFramebufferSize(const int width, const int height)
{
	this->width = width;
	this->height = height;

	printf("Framebuffer Width: %i\n", this->width);
	printf("Framebuffer Height: %i\n", this->height);

	glViewport(0, 0, width, height);

	// Update projection matrix
	// TODO
}

void Renderer::setClearColor(const vec4& color)
{
	clearColor = color;

	glClearColor(
		clearColor.x,
		clearColor.y,
		clearColor.z,
		clearColor.w
	);
}