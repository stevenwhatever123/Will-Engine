#version 450 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec4 tangent;
layout(location = 3) in vec4 bitangent;
layout(location = 4) in vec2 texCoord;

// BRDF: 0. Emissive, 1. Albedo (Diffuse), 2. Metallic, 3. Roughness
// Others: 4. Normal Map
layout(set = 1, binding = 1) uniform sampler2D texColor[5];

layout(location = 0) out vec4 GBuffer0;
layout(location = 1) out vec4 GBuffer1;
layout(location = 2) out vec4 GBuffer2;
layout(location = 3) out vec4 GBuffer3;

void main()
{
	vec4 oPosition = position;

	vec3 oNormal = normalize(normal.rgb);
	vec3 tNormal = vec3(texture(texColor[4], texCoord));
	// The B channel of a normal map must always be in between 128 and 255. Any value below 128 is an unvalid normal map
	if(tNormal.b * 255 >= 128 && tNormal.b * 255 <= 255)
	{
		// Converting the value from [0,1] back to [-1,1]
		tNormal = normalize(tNormal * 2.0 - 1.0);

		// Convert normal from texture from tangent space to world space
		vec3 tangent_temp = normalize(tangent.rgb);

		vec3 bitangent_temp = normalize(bitangent.rgb);

		vec3 normal_temp = normalize(normal.rgb);

		mat3 TBN = mat3(tangent_temp, bitangent_temp, normal_temp);

		oNormal = normalize(TBN * tNormal);
	}
	

	float lod = textureQueryLod(texColor[0], texCoord).x;
	vec4 tEmissive = texture(texColor[0], texCoord);

	lod = textureQueryLod(texColor[1], texCoord).x;
	vec4 tAlbedo = texture(texColor[1], texCoord);

	lod = 30;
	float tMetallic = texture(texColor[2], texCoord).r;

	lod = 30;
	float tRoughness = texture(texColor[3], texCoord).r;

	GBuffer0 = vec4(vec3(oPosition), tMetallic);
	GBuffer1 = vec4(vec3(oNormal), tRoughness);
	GBuffer2 = vec4(vec3(tEmissive), 1);
	GBuffer3 = vec4(vec3(tAlbedo), 1);
}