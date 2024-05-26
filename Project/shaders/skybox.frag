#version 450
layout(push_constant) uniform constants
{
	mat4 model;
} push;


layout(set = 1, binding = 1) uniform samplerCube samplerCubeMap;


layout (location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = texture(samplerCubeMap, inUVW);
}