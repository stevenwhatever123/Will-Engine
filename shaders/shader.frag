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
	float intensity;
};

layout(set = 2, binding = 0) uniform camera
{
	vec4 cameraPosition;
};

layout(set = 3, binding = 1) uniform sampler2D texColor[4];

layout(location = 0) out vec4 oColor;

void main()
{
	vec4 emissiveColor = texture(texColor[0], texCoord);
	vec4 ambientColor = texture(texColor[1], texCoord);
	vec4 diffuseColor = texture(texColor[2], texCoord);
	vec4 specularColor = texture(texColor[3], texCoord);

	// Emissive
	vec4 emissive = emissiveColor;

	// Ambient
    vec4 ambient = lightAmbient * ambientColor;

	vec4 lightDirection = normalize(lightPosition - position);
	vec4 viewDirection = normalize(cameraPosition - position);
	vec4 halfVector = normalize(lightDirection + viewDirection);

	// Diffuse
	float lightCosTheta = max(0, dot(normalize(normal), normalize(lightDirection)));
	vec4 diffuse = lightColor * diffuseColor * lightCosTheta;

	// Specular
	float halfVectorCosTheta = max(0, dot(normalize(normal), normalize(halfVector)));
	vec4 specular = max(vec4(0, 0, 0, 1), specularColor * lightColor * pow(halfVectorCosTheta, max(0, 1 / intensity)));

    vec4 result = emissive + ambient + diffuse + specular;

	oColor = result;
}