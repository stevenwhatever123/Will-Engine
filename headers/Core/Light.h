#pragma once
#include "Core/UniformClass.h"

class Light
{
public:

	vec3 position;

	// For Point Light Shadow mapping
	mat4 matrices[6];
	
	LightUniform lightUniform;

public:

	Light();
	Light(vec3 position);
	Light(vec3 position, vec4 color);
	~Light();

	void update();

private:
};

