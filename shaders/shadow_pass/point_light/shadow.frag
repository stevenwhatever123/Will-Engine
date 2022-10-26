#version 450 core

layout(location = 0) in vec4 position;

layout(set = 1, binding = 1) uniform light
{
	vec4 lightPosition;
	vec4 lightColor;
	vec4 lightAmbient;
	float intensity;
};

void main()
{
	float lightDistance = length(vec3(position - lightPosition));

	lightDistance = lightDistance / 10000.0f;

	gl_FragDepth = lightDistance;
}