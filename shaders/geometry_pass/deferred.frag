#version 450 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 texCoord;

layout(set = 1, binding = 1) uniform sampler2D texColor[5];

layout(location = 0) out vec4 oPosition;
layout(location = 1) out vec4 oNormal;
layout(location = 2) out vec4 oEmissive;
layout(location = 3) out vec4 oAmbient;
layout(location = 4) out vec4 oAlbedo;
layout(location = 5) out vec4 oMetallic;
layout(location = 6) out vec4 oRoughness;

void main()
{
	oPosition = position;
	oNormal = normalize(normal);

	float lod = textureQueryLod(texColor[0], texCoord).x;
	oEmissive = texture(texColor[0], texCoord, lod);

	lod = textureQueryLod(texColor[1], texCoord).x;
	oAmbient = texture(texColor[1], texCoord, lod);

	lod = textureQueryLod(texColor[2], texCoord).x;
	oAlbedo = texture(texColor[2], texCoord, lod);

	lod = textureQueryLod(texColor[3], texCoord).x;
	oMetallic = vec4(vec3(texture(texColor[3], texCoord, lod).r), 1);

	lod = textureQueryLod(texColor[4], texCoord).x;
	oRoughness = vec4(vec3(texture(texColor[4], texCoord, lod).r), 1);
}