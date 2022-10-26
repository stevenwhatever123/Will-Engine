#pragma once

struct CameraMatrix
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

struct LightUniform
{
	vec4 transformedPosition;
	vec4 color;
	vec4 ambient;
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