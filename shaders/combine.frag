#version 450
#extension GL_KHR_vulkan_glsl: enable

layout (location = 0) in vec2 texCoord;

layout(set = 0, binding = 0) uniform sampler2D texColor[7];


layout (location = 0) out vec4 oColor;

void main()
{
	oColor = texture(texColor[4], texCoord);
}