#version 450
#extension GL_KHR_vulkan_glsl: enable

layout (location = 0) in vec2 texCoord;

layout(set = 0, binding = 1) uniform light
{
	vec4 lightPosition;
	vec4 lightColor;
	float lightRange;
	float intensity;
};

layout(set = 1, binding = 1) uniform camera
{
	vec4 cameraPosition;
};

layout(set = 2, binding = 2) uniform sampler2D texColor[5];

layout(set = 3, binding = 3) uniform samplerCube depthMap;

layout (location = 0) out vec4 oColor;

// Taking inspiration from GPU Gems: Shadow Map Antialiasing
vec4 offset_lookup(vec3 texCoord, vec3 offset, float texmapscale)
{
	return texture(depthMap, texCoord + offset * texmapscale);
}

float ShadowCalculation(vec3 position, vec3 normal)
{
	vec3 fragToLight = position - vec3(lightPosition);

	float closestDepth = texture(depthMap, fragToLight).r;

	closestDepth *= 2000.0f;

	float currentDepth = length(fragToLight);

	float elipson = max(0.05 * (1.0 - dot(vec3(normal), fragToLight)), 0.005); 

	vec2 texmapscale = 1 / textureSize(depthMap, 0);
	float sum = 0;

	for(float x = -1.5; x <= 1.5; x += 1.0)
	{
		for(float y = -1.5; y <= 1.5; y += 1.0)
		{
			for(float z = -1.5; z <= 1.5; z += 1.0)
			{
				closestDepth = offset_lookup(fragToLight, vec3(x, y, z), texmapscale.x).r;
				closestDepth *= 2000;

				sum = (currentDepth - elipson > closestDepth)? sum + closestDepth / 2000 : sum;
			}
		}
	}

	float shadow = sum / 16.0;

    return shadow;
}

// The following BRDF code references from: https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf

// Specular D - GGX/Trowbridge-Reitz
// For normal distribution
float GGXTrowbridgeReitz(vec3 normal, vec3 halfVector, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float NdotH = max(0.001, dot(normal, halfVector));
	float NdotHSq = NdotH * NdotH;

	return alphaSq / (radians(180) * pow((NdotHSq * (alphaSq - 1) + 1), 2));
}

// Helper function for Schlick-Beckmann Model - G1
float G1(vec3 normal, vec3 direction, float roughness)
{
	float k = pow(roughness + 1, 2) / 8.0;

	float NdotD = max(0.001, dot(normal, direction));

	return NdotD / (NdotD * (1 - k) + k);
}

// Specular G - Schlick-Beckmann Model
// For geometric attenuation
float SchlickBeckmannModel(vec3 normal, vec3 viewDirection, vec3 lightDirection, float roughness)
{
	return G1(normal, lightDirection, roughness) * G1(normal, viewDirection, roughness);
}

// Specular F - Schlick’s approximation
// For fresnel
vec4 FresenlSchlick(vec4 dielectricSpecular, vec3 viewDirection, vec3 halfVector)
{
	// Schlick’s approximation
	// https://en.wikipedia.org/wiki/Schlick%27s_approximation

	// Pre-compute f0
	// https://learnopengl.com/pbr/theory
	vec4 f0 = dielectricSpecular;

	float VdotH = max(0.001, dot(viewDirection, halfVector));
	float power = ((-5.55473 * VdotH) -6.98316) * VdotH;

	return f0 + (1 - f0) * pow(2, power);
}

void main()
{
	const vec3 position = vec3(vec4(texture(texColor[0], texCoord).rgb, 1));
	const vec3 normal = vec3(vec4(normalize(texture(texColor[1], texCoord)).rgb, 1));
	const vec4 emissive = texture(texColor[2], texCoord);
	const vec4 ambient = texture(texColor[3], texCoord);
	const vec4 albedo = texture(texColor[4], texCoord);
	const float metallic = texture(texColor[0], texCoord).a;
	const float roughness = texture(texColor[1], texCoord).a;

	// Values we are going to use later
	const vec3 lightDirection = normalize(vec3(lightPosition) - position);
	const vec3 viewDirection = normalize(vec3(cameraPosition) - position);
	const vec3 halfVector = normalize(lightDirection + viewDirection);
	const vec4 dielectricSpecular = vec4(0.04, 0.04, 0.04, 1);
	const vec4 black = vec4(0, 0, 0, 1);

	// BRDF Diffuse
	vec4 diffuse = albedo / radians(180);

	// BRDF Specular

	float NdotL = dot(normal, lightDirection);
	float NdotV = dot(normal, viewDirection);
	float NdotH = dot(normal, halfVector);
	float VdotH = dot(viewDirection, halfVector);

	float D = GGXTrowbridgeReitz(normal, halfVector, roughness);
	float G = SchlickBeckmannModel(normal, viewDirection, lightDirection, roughness);
	vec4 F = FresenlSchlick(dielectricSpecular, viewDirection, halfVector);

	vec4 ks = F;
	vec4 kd = 1 - ks;

	vec4 specular = (D * F * G) / (4 * max(0.001, NdotL * NdotV));

	if(NdotL < 0 || NdotV < 0)
		specular = vec4(0, 0, 0, 1);

	float lightDistance = abs(distance(vec3(lightPosition), position));
	float attenuation = lightRange / (lightDistance * lightDistance);
	vec4 brightness = lightColor * intensity * attenuation;

	float shadow = ShadowCalculation(position, normal);

	vec4 brdf = (kd * diffuse + specular) * brightness * max(NdotL, 0.0);

    //vec4 result = emissive + ambient + (1.0 - shadow) * brdfResult;
    vec4 result = emissive + ambient + brdf;
    result = emissive + ambient + (1.0 - shadow) * brdf;

	oColor = result;
}