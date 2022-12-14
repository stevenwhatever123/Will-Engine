#include "pch.h"
#include "Core/Light.h"

u32 Light::idCounter = 0;

Light::Light() :
    id(++idCounter),
    lastPosition(0),
    currentPosition(0),
    renderShadow(true),
    matrices(),
    lightUniform({ vec4(0, 0, 0, 1), vec4(1), 1, 1 })
{

}

Light::Light(vec3 position) :
    id(++idCounter),
    lastPosition(0),
    currentPosition(position),
    renderShadow(true),
    matrices(),
    lightUniform({ vec4(0, 0, 0, 1), vec4(1), 1, 1 })
{

}

Light::Light(vec3 position, vec4 color) :
    id(++idCounter),
    lastPosition(0),
    currentPosition(position),
    renderShadow(true),
    matrices(),
    lightUniform({ vec4(0, 0, 0, 1), color, 1, 1 })
{

}

Light::~Light()
{

}

void Light::update()
{
    if (glm::abs(glm::length(currentPosition) - glm::length(lastPosition)) > 0.001f)
    {
        renderShadow = true;
    }

    lastPosition = currentPosition;

    if (!renderShadow)
        return;

    updateLightUniform();

    // View projection matrices for 6 different side of the cube map
    // Order: +x, -x, +y, -y, +z, -z
    mat4 lightProjectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 2000.0f);

    matrices[0] = lightProjectionMatrix * glm::lookAt(currentPosition, currentPosition + vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
    matrices[1] = lightProjectionMatrix * glm::lookAt(currentPosition, currentPosition + vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
    matrices[2] = lightProjectionMatrix * glm::lookAt(currentPosition, currentPosition + vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
    matrices[3] = lightProjectionMatrix * glm::lookAt(currentPosition, currentPosition + vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f));
    matrices[4] = lightProjectionMatrix * glm::lookAt(currentPosition, currentPosition + vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f));
    matrices[5] = lightProjectionMatrix * glm::lookAt(currentPosition, currentPosition + vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f));
}


void Light::updateLightUniform()
{
    lightUniform.transformedPosition = vec4(currentPosition.x, currentPosition.y, currentPosition.z, 1);
}

void Light::updateLightPosition(vec3 position)
{
    currentPosition = position;
}

void Light::shadowRendered()
{
    renderShadow = false;
}

bool Light::shouldRenderShadow() const
{
    return renderShadow;
}