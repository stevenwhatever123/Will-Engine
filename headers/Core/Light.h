#pragma once
#include "Core/UniformClass.h"

class Light
{
public:

	vec3 lastPosition;
	vec3 position;

	bool renderShadow;

	bool renderGui;

	// For Point Light Shadow mapping
	mat4 matrices[6];
	
	LightUniform lightUniform;

public:

	Light();
	Light(vec3 position);
	Light(vec3 position, vec4 color);
	~Light();

	void update();

	// Command call
	void shadowRendered();

	// Getters
	bool shouldRenderShadow() const;

private:
};

