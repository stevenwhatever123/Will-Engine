#pragma once
#include "Core/UniformClass.h"

class Light
{
public:

	const u32 id;

	vec3 lastPosition;
	vec3 currentPosition;

	bool renderShadow;
	f32 range;

	// For Point Light Shadow mapping
	mat4 matrices[6];

	LightUniform lightUniform;

private:

	static u32 idCounter;

public:

	Light();
	Light(vec3 position);
	Light(vec3 position, vec4 color);
	~Light();

	void update();

	void updateLightUniform();

	void updateLightPosition(vec3 position);

	// Command call
	void shadowRendered();

	// Setters
	void needRenderShadow() { renderShadow = true; }
	// Getters
	bool shouldRenderShadow() const;

private:
};