#include "pch.h"
#include "Core/Light.h"

Light::Light() :
	position(0),
	lightUniform({vec4(0, 0, 0, 1), vec4(1), vec4(0.02f, 0.02f, 0.02f, 1), 1})
{

}

Light::Light(vec3 position):
	position(position),
	lightUniform({ vec4(0, 0, 0, 1), vec4(1), vec4(0.02f, 0.02f, 0.02f, 1), 1 })
{

}

Light::Light(vec3 position, vec4 color):
	position(position),
	lightUniform({ vec4(0, 0, 0, 1), color, vec4(0.02f, 0.02f, 0.02f, 1), 1 })
{

}

Light::~Light()
{

}