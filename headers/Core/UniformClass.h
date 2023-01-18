#pragma once
#define MAX_BONES 256

struct CameraMatrix
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

struct BoneUniform
{
	mat4 boneMatrices[MAX_BONES];
};

struct LightUniform
{
	vec4 transformedPosition;
	vec4 color;
	f32 range;
	f32 intensity;
};

struct PhongMaterialUniform
{
	// std140 layout
	vec4 emissiveColor;
	vec4 ambientColor;
	vec4 diffuseColor;
	vec4 specularColor;
};

// Metallic-Roughness Material model 
struct BRDFMetallic
{
	vec4 emissive;
	vec4 ambient;
	vec4 albedo;
	f32 metallic;
	f32 roughness;
};

// Specular-Glossiness Material model
struct BRDFSpecular
{
	// Leave this for now, I'm going to work on this later
	vec4 albedo;
	vec4 specular;
	f32 glossiness;
};

struct PushConstantModelInfo 
{
	mat4 modelTransform;
	//u32 materialIndex;
};