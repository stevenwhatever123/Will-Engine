#version 450 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 texCoord;

layout(set = 1, binding = 0) uniform light
{
	vec4 lightPosition;
};

layout(set = 2, binding = 0) uniform camera
{
	vec4 cameraPosition;
};

layout(set = 3, binding = 1) uniform sampler2D texColor;

layout(location = 0) out vec4 oColor;

void main()
{
	//oColor = texture(texColor, texCoord);

	vec4 lightColor = vec4(1, 1, 1, 1);

	// Ambient
	float ambientStrength = 0.1;
    vec4 ambient = ambientStrength * lightColor;

	// Diffuse
	vec4 lightDirection = normalize(lightPosition - position);
	float diff = max(dot(normalize(normal), normalize(lightDirection)), 0.0);
	vec4 diffuse = diff * lightColor;

	// Specular
//	float specularStrength = 0.5;
//	vec4 viewDirection = normalize(cameraPosition - position);
//	vec4 reflectDirection = reflect(-lightDirection, normal);
//	float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32);
//	vec4 specular = specularStrength * spec * lightColor;  

	vec4 objectColor = texture(texColor, texCoord);
    vec4 result = (diffuse) * objectColor;

	oColor = result;
}