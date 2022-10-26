#version 450 core
layout(triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout(set = 0, binding = 2) uniform lightMatrices
{
	// This includes projection and view
	mat4 shadowMatrix[6];
};

layout(location = 0) out vec4 oPosition;

void main()
{
	for(int face = 0; face < 6; face++)
	{
		gl_Layer = face;
		for(int i = 0; i < 3; i++)
		{
			oPosition = gl_in[i].gl_Position;
			gl_Position = shadowMatrix[face] * oPosition;
			EmitVertex();
		}
		EndPrimitive();
	}
}