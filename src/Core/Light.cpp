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

void Light::update()
{
    // View projection matrices for 6 different side of the cube map
    // Order: +x, -x, +y, -y, +z, -z
    mat4 lightProjectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10000.0f);
    //lightProjectionMatrix[1][1] *= -1.0f;

    //matrices[0] = lightProjectionMatrix * glm::lookAt(position, position + vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    //matrices[1] = lightProjectionMatrix * glm::lookAt(position, position + vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    //matrices[2] = lightProjectionMatrix * glm::lookAt(position, position + vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
    //matrices[3] = lightProjectionMatrix * glm::lookAt(position, position + vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f));
    //matrices[4] = lightProjectionMatrix * glm::lookAt(position, position + vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f));
    //matrices[5] = lightProjectionMatrix * glm::lookAt(position, position + vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));

    matrices[0] = lightProjectionMatrix * glm::lookAt(position, position + vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
    matrices[1] = lightProjectionMatrix * glm::lookAt(position, position + vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
    matrices[2] = lightProjectionMatrix * glm::lookAt(position, position + vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
    matrices[3] = lightProjectionMatrix * glm::lookAt(position, position + vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f));
    matrices[4] = lightProjectionMatrix * glm::lookAt(position, position + vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f));
    matrices[5] = lightProjectionMatrix * glm::lookAt(position, position + vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f));
}