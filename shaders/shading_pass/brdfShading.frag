#version 450
#extension GL_KHR_vulkan_glsl: enable

layout (location = 0) in vec2 texCoord;

layout(set = 0, binding = 1) uniform light
{
	vec4 lightPosition;
	vec4 lightColor;
	vec4 lightAmbient;
	float intensity;
};

layout(set = 1, binding = 1) uniform camera
{
	vec4 cameraPosition;
};

layout(set = 2, binding = 2) uniform sampler2D texColor[7];

layout(set = 3, binding = 3) uniform samplerCube depthMap;

layout (location = 0) out vec4 oColor;

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

float ShadowCalculation(vec4 position)
{
	vec3 fragToLight = vec3(position - lightPosition);

	float closestDepth = texture(depthMap, fragToLight).r;

	closestDepth *= 2000.0f;

	float currentDepth = length(fragToLight);

	float elipson = 0.005f; 

    float shadow = currentDepth + elipson > closestDepth? 1.0 : 0.0;

    return shadow;
}

void main()
{
	const vec4 position = texture(texColor[0], texCoord);
	const vec4 normal = normalize(texture(texColor[1], texCoord));
	const vec4 emissive = texture(texColor[2], texCoord);
	const vec4 ambient = texture(texColor[3], texCoord);
	const vec4 albedo = texture(texColor[4], texCoord);
	const float metallic = texture(texColor[5], texCoord).x;
	const float roughness = texture(texColor[6], texCoord).x;

	// Values we are going to use later
	const vec4 lightDirection = normalize(lightPosition - position);
	const vec4 viewDirection = normalize(cameraPosition - position);
	const vec4 halfVector = normalize(lightDirection + viewDirection);
	const vec4 dielectricSpecular = vec4(0.04, 0.04, 0.04, 1);
	const vec4 black = vec4(0, 0, 0, 1);

	float lightDistance = abs(distance(lightPosition, position));

	// BRDF Specular from Cook-Torrance
	// Using the formula of (F(l,h) * G(l,v,h) * D(h)) / (4 * (n*l) * (n * v))
	// F - Fresnel function
	vec4 f0 = mix(dielectricSpecular, albedo, metallic);
	vec4 f = f0 + (1 - f0) * pow((1 - dot(viewDirection, halfVector)), 5);

	// G - Geometric occlusion
	float g = g1(normal, lightDirection, roughness) * g1(normal, viewDirection, roughness);

	// D - The normal distribution
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;
	float normalDotHalfVector = max(0, dot(normal, halfVector));
	float normalDotHalfVectorSq = normalDotHalfVector * normalDotHalfVector;
	float d = alphaSq / (radians(180) * (pow(normalDotHalfVectorSq * (alpha - 1) + 1, 2)));

	// The result of BRDF specular
	vec4 specular = (f * g * d) / (4 * max(0, dot(normal, lightDirection)) * max(0, dot(normal, viewDirection)) + 0.0001);
	vec4 brdfSpecular = specular;

	// BRDF Diffuse
	// Using the formula of (1 - F(v*h))(cDiff / pi)
	vec4 cDiff = mix(albedo * (1 - dielectricSpecular.x), black, metallic);
	vec4 diffuse = (1 - f) * (cDiff / radians(180));
	vec4 brdfDiffuse = diffuse;

	vec4 brdfResult = (diffuse + specular) * lightColor * intensity * max(0, dot(normal, lightDirection));

	float shadow = ShadowCalculation(position);

    vec4 result = emissive + ambient + (1.0 - shadow) * brdfResult;

	oColor = result;
}