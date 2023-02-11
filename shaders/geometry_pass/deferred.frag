#version 450 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 texCoord;

layout(set = 1, binding = 1) uniform sampler2D texColor[4];

layout(location = 0) out vec4 GBuffer0;
layout(location = 1) out vec4 GBuffer1;
layout(location = 2) out vec4 GBuffer2;
layout(location = 3) out vec4 GBuffer3;

void main()
{
	vec4 tPosition = position;
	vec4 tNormal = normalize(normal);

	float lod = textureQueryLod(texColor[0], texCoord).x;
	//vec4 tEmissive = texture(texColor[0], texCoord, lod);
	vec4 tEmissive = texture(texColor[0], texCoord);

	lod = textureQueryLod(texColor[1], texCoord).x;
	//vec4 tAlbedo = texture(texColor[1], texCoord, lod);
	vec4 tAlbedo = texture(texColor[1], texCoord);

	// As the texture of metallic and roughness map is using the same value for all pixels. Sample the texture using min lod to save performance
	// 30 should be enough to select the lowest lod for all textures in different resolution
	lod = 30;
	//float tMetallic = texture(texColor[2], texCoord, lod).r;
	float tMetallic = texture(texColor[2], texCoord).r;

	lod = 30;
	//float tRoughness = texture(texColor[3], texCoord, lod).r;
	float tRoughness = texture(texColor[3], texCoord).r;

	GBuffer0 = vec4(vec3(tPosition), tMetallic);
	GBuffer1 = vec4(vec3(tNormal), tRoughness);
	GBuffer2 = vec4(vec3(tEmissive), 1);
	GBuffer3 = vec4(vec3(tAlbedo), 1);
}