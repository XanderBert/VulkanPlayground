#version 450

layout(push_constant) uniform constants
{
    mat4 model;
} push;

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 viewProjection;
	vec4 viewPos;
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUV;

layout (location = 0) out vec3 outUVW;

void main() 
{
	outUVW = inPos;
	// Convert cubemap coordinates into Vulkan coordinate space
	outUVW.xy *= -1.0;
	// Remove translation from view matrix
	mat4 viewMat = mat4(mat3(push.model));
	gl_Position = ubo.viewProjection * viewMat * vec4(inPos.xyz, 1.0);
}
