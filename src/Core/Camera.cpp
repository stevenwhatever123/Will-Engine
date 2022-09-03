#include "pch.h"
#include "Core/Camera.h"

Camera::Camera(vec3 pos, vec3 lookAt):
	position(pos),
	lookAt(lookAt),
	fov(90),
	forward(glm::normalize(lookAt - pos)),
	up(glm::normalize(vec3(0, 1, 0))),
	cameraMatrix(1)
{

}

Camera::~Camera()
{

}

void Camera::moveForward(const f32 value)
{
	position += forward * value;
}

void Camera::moveRight(const f32 value)
{
	// Get the right direction using the cross product
	vec3 v = glm::normalize(glm::cross(glm::normalize(forward), glm::normalize(up))) * value;

	position += v;
}

void Camera::moveUp(const f32 value)
{
	position += up * value;
}

void Camera::updateCameraMatrix()
{
	cameraMatrix = glm::lookAt(position, position + forward, up);
}

mat4 Camera::getCameraMatrix() const
{
	return cameraMatrix;
}

mat4 Camera::getProjectionMatrix(i32 width, i32 height) const
{
	mat4 projectionMatrix = glm::perspective(glm::radians(fov), (f32)width / (f32)height, 0.1f, 1000.0f);

	// Flip y axis for vulkan
	projectionMatrix[1][1] *= -1.0f;

	return projectionMatrix;
}