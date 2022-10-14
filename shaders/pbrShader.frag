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

layout(set = 3, binding = 1) uniform sampler2D texColor[5];

layout(location = 0) out vec4 oColor;

// The following code mainly following the slides from:
// https://www.khronos.org/assets/uploads/developers/library/2017-web3d/glTF-2.0-Launch_Jun17.pdf

float g1(vec4 normal, vec4 direction, float roughness)
{
	float alpha = pow(roughness, 2);
	float alphaSq = pow(alpha, 2);

	float normalDotDirection = max(0, dot(normal, direction));

	float result = (2 * normalDotDirection) / (normalDotDirection + sqrt(alphaSq + (1 - alphaSq) * pow(normalDotDirection, 2)));

	return result;
}

void main()
{
	vec4 emissiveColor = texture(texColor[0], texCoord);
	vec4 ambientColor = texture(texColor[1], texCoord);
	vec4 albedo = texture(texColor[2], texCoord);
	float metallic = texture(texColor[3], texCoord).x;
	float roughness = texture(texColor[4], texCoord).x;

	// Discard this fragment if albedo's alpha is set to 0
	if(albedo.w < 0.01)
		discard;

	// Emissive
	vec4 emissive = emissiveColor;

	// Ambient
    vec4 ambient = lightAmbient * ambientColor;

	// Values we are going to use later
	const vec4 normalizedNormal = normalize(normal);
	const vec4 lightDirection = normalize(lightPosition - position);
	const vec4 viewDirection = normalize(cameraPosition - position);
	const vec4 halfVector = normalize(lightDirection + viewDirection);
	const vec4 dielectricSpecular = vec4(0.04, 0.04, 0.04, 1);
	const vec4 black = vec4(0, 0, 0, 1);

	// BRDF Specular from Cook-Torrance
	// Using the formula of (F(l,h) * G(l,v,h) * D(h)) / (4 * (n*l) * (n * v))
	// F - Fresnel function
	vec4 f0 = mix(dielectricSpecular, albedo, metallic);
	vec4 f = f0 + (1 - f0) * pow((1 - dot(viewDirection, halfVector)), 5);

	// G - Geometric occlusion
	float g = g1(normalizedNormal, lightDirection, roughness) * g1(normalizedNormal, viewDirection, roughness);

	// D - The normal distribution
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;
	float normalDotHalfVector = max(0, dot(normalizedNormal, halfVector));
	float normalDotHalfVectorSq = normalDotHalfVector * normalDotHalfVector;
	float d = alphaSq / (radians(180) * (pow(normalDotHalfVectorSq * (alpha - 1) + 1, 2)));

	// The result of BRDF specular
	vec4 specular = (f * g * d) / (4 * max(0, dot(normalizedNormal, lightDirection)) * max(0, dot(normalizedNormal, viewDirection)) + 0.0001);
	vec4 brdfSpecular = specular;

	// BRDF Diffuse
	// Using the formula of (1 - F(v*h))(cDiff / pi)
	vec4 cDiff = mix(albedo * (1 - dielectricSpecular.x), black, metallic);
	vec4 diffuse = (1 - f) * (cDiff / radians(180));
	vec4 brdfDiffuse = diffuse;

	vec4 brdfResult = (diffuse + specular) * lightColor * intensity * max(0, dot(normalizedNormal, lightDirection));

    vec4 result = emissive + ambient + brdfResult;

	oColor = result;
}