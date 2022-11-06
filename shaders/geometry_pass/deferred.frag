#version 450 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 texCoord;

layout(set = 1, binding = 1) uniform sampler2D texColor[5];

layout(location = 0) out vec4 GBuffer0;
layout(location = 1) out vec4 GBuffer1;
layout(location = 2) out vec4 GBuffer2;
layout(location = 3) out vec4 GBuffer3;
layout(location = 4) out vec4 GBuffer4;

void main()
{
	vec4 tPosition = position;
	vec4 tNormal = normalize(normal);

	float lod = textureQueryLod(texColor[0], texCoord).x;
	vec4 tEmissive = texture(texColor[0], texCoord, lod);

	lod = textureQueryLod(texColor[1], texCoord).x;
	vec4 tAmbient = texture(texColor[1], texCoord, lod);

	lod = textureQueryLod(texColor[2], texCoord).x;
	vec4 tAlbedo = texture(texColor[2], texCoord, lod);

	lod = textureQueryLod(texColor[3], texCoord).x;
	float tMetallic = texture(texColor[3], texCoord, lod).r;

	lod = textureQueryLod(texColor[4], texCoord).x;
	float tRoughness = texture(texColor[4], texCoord, lod).r;

	GBuffer0 = vec4(vec3(tPosition), tMetallic);
	GBuffer1 = vec4(vec3(tNormal), tRoughness);
	GBuffer2 = vec4(vec3(tEmissive), 1);
	GBuffer3 = vec4(vec3(tAmbient), 1);
	GBuffer4 = vec4(vec3(tAlbedo), 1);
}