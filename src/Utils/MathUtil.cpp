#include "pch.h"
#include "Utils/MathUtil.h"

#include <glm/gtx/matrix_decompose.hpp>

using namespace WillEngine;

mat4 WillEngine::Utils::AssimpMat4ToGlmMat4(const aiMatrix4x4 aiMatrix)
{
	mat4 glmMatrix(1);

	//glmMatrix[0][0] = aiMatrix[0][0];
	//glmMatrix[1][0] = aiMatrix[0][1];
	//glmMatrix[2][0] = aiMatrix[0][2];
	//glmMatrix[3][0] = aiMatrix[0][3];

	//glmMatrix[0][1] = aiMatrix[1][0];
	//glmMatrix[1][1] = aiMatrix[1][1];
	//glmMatrix[2][1] = aiMatrix[1][2];
	//glmMatrix[3][1] = aiMatrix[1][3];

	//glmMatrix[0][2] = aiMatrix[2][0];
	//glmMatrix[1][2] = aiMatrix[2][1];
	//glmMatrix[2][2] = aiMatrix[2][2];
	//glmMatrix[3][2] = aiMatrix[2][3];

	//glmMatrix[0][3] = aiMatrix[3][0];
	//glmMatrix[1][3] = aiMatrix[3][1];
	//glmMatrix[2][3] = aiMatrix[3][2];
	//glmMatrix[3][3] = aiMatrix[3][3];

	//=================================

	glmMatrix[0][0] = aiMatrix.a1;
	glmMatrix[1][0] = aiMatrix.a2;
	glmMatrix[2][0] = aiMatrix.a3;
	glmMatrix[3][0] = aiMatrix.a4;

	glmMatrix[0][1] = aiMatrix.b1;
	glmMatrix[1][1] = aiMatrix.b2;
	glmMatrix[2][1] = aiMatrix.b3;
	glmMatrix[3][1] = aiMatrix.b4;

	glmMatrix[0][2] = aiMatrix.c1;
	glmMatrix[1][2] = aiMatrix.c2;
	glmMatrix[2][2] = aiMatrix.c3;
	glmMatrix[3][2] = aiMatrix.c4;

	glmMatrix[0][3] = aiMatrix.d1;
	glmMatrix[1][3] = aiMatrix.d2;
	glmMatrix[2][3] = aiMatrix.d3;
	glmMatrix[3][3] = aiMatrix.d4;
	
	return glmMatrix;
}

void WillEngine::Utils::DecomposeMatrix(mat4 in, vec3& position, vec3& rotation, vec3& scale)
{
	// Dummy values
	vec3 translation;
	quat quatRotation;
	vec3 scaling;
	vec3 skew;
	vec4 perspective;

	glm::decompose(in, scaling, quatRotation, translation, skew, perspective);

	// Convert rotation from quaternion into euler angles
	glm::extractEulerAngleXYZ(glm::toMat4(quatRotation), rotation.x, rotation.y, rotation.z);

	position = translation;
	scale = scaling;
}