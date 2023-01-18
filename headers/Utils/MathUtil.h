#pragma once

namespace WillEngine::Utils
{
	mat4 AssimpMat4ToGlmMat4(const aiMatrix4x4 aiMatrix);

	void DecomposeMatrix(mat4 in, vec3& position, vec3& rotation, vec3& scale);
}