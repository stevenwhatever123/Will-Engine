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
	//oNormal = normalize(normal) * 0.5 + 0.5; // transforming from [-1,1] to [0,1]
	oNormal = normalize(normal);
	oEmissive = texture(texColor[0], texCoord);
	oAmbient = texture(texColor[1], texCoord);
	oAlbedo = texture(texColor[2], texCoord);
	oMetallic = vec4(texture(texColor[3], texCoord).x, texture(texColor[3], texCoord).x, texture(texColor[3], texCoord).x, 1);
	oRoughness = vec4(texture(texColor[4], texCoord).x, texture(texColor[3], texCoord).x, texture(texColor[3], texCoord).x, 1);
}