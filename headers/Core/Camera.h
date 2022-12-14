#pragma once
class Camera
{
public:
	
	vec3 position;
	vec3 rotation;

private:

	vec3 lookAt;

	// Field of view
	f32 fov;

	vec3 forward;
	vec3 up;

	// The view matrix
	mat4 cameraMatrix;

public:

	Camera(vec3 pos, vec3 lookAt);
	~Camera();

	void moveForward(const f32 value);
	void moveRight(const f32 value);
	void moveUp(const f32 value);

	void rotate(const f32 xValue, const f32 yValue, const f32 zValue);

	void updateCameraMatrix();
	mat4 getCameraMatrix() const;

	// Use the framebuffer size for finding the perspective matrix
	mat4 getProjectionMatrix(i32 width, i32 height) const;
};