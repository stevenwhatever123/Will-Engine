#pragma once
struct LightUniform
{
	vec4 transformedPosition;
	vec4 color;
	vec4 ambient;
	f32 intensity;
};

class Light
{
public:

	vec3 position;
	
	LightUniform lightUniform;

public:

	Light();
	Light(vec3 position);
	Light(vec3 position, vec4 color);
	~Light();

private:
};

