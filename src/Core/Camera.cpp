#include "pch.h"
#include "Core/Camera.h"

Camera::Camera(vec3 pos, vec3 lookAt):
	position(pos),
	positionShift(0),
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

void Camera::rotate(const f32 xValue, const f32 yValue, const f32 zValue)
{
	glm::mat4 rotationMatrixX = glm::rotate(mat4(1), glm::radians(xValue), glm::normalize(glm::cross(glm::normalize(forward), glm::normalize(up))));
	forward = glm::vec3(rotationMatrixX * glm::vec4(forward, 1));

	glm::mat4 rotationMatrixY = glm::rotate(mat4(1), glm::radians(yValue), glm::normalize(glm::normalize(up)));
	forward = glm::vec3(rotationMatrixY * glm::vec4(forward, 1));

	glm::mat4 rotationMatrixZ = glm::eulerAngleZ(glm::radians(zValue));
	up = glm::vec3(rotationMatrixZ * glm::vec4(up, 1));
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