#version 450 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 texCoord;

layout(set = 1, binding = 0) uniform light
{
	vec4 lightPosition;
	vec4 lightColor;
	vec4 lightAmbient;
};

layout(set = 2, binding = 0) uniform camera
{
	vec4 cameraPosition;
};

layout(set = 3, binding = 1) uniform sampler2D texColor;

layout(push_constant, std140) uniform materialUniform
{
	// std140 layout
	layout(offset = 64) vec4 emissiveColor;
	layout(offset = 80) vec4 ambientColor;
	layout(offset = 96) vec4 diffuseColor;
	layout(offset = 112) vec4 specularColor;
};

layout(location = 0) out vec4 oColor;

void main()
{
	// Emissive
	vec4 emissive = emissiveColor;

	// Ambient
    vec4 ambient = lightAmbient * ambientColor;

	// Specular
	vec4 lightDirection = normalize(lightPosition - position);
	vec4 viewDirection = normalize(cameraPosition - position);

	float specularStrength = 0.5;
	vec4 biSector = (lightDirection + viewDirection) / 2;
	float biSectorCosTheta = max(0, dot(normalize(normal), normalize(biSector)));
	vec4 specular = diffuseColor * lightColor * pow(biSectorCosTheta, specularStrength);

	// Diffuse
	float lightCosTheta = max(0, dot(normalize(normal), normalize(lightDirection)));
	vec4 diffuse = lightColor * diffuseColor * lightCosTheta;

	vec4 objectColor = texture(texColor, texCoord);
    //vec4 result = (emissive + ambient + diffuse + specular) * objectColor;
    vec4 result = objectColor;

	oColor = result;
}