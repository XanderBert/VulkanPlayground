#version 450

layout(push_constant) uniform constants
{
    mat4 model;
} push;

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 viewProjection;
	vec4 viewPos;
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outViewDirection;
layout(location = 3) out vec4 outWorldPos;


void main() 
{	
    outNormal = mat3(push.model)*  inNormal ;
	outUV = inUV;

	gl_Position = ubo.viewProjection * push.model * vec4(inPos, 1.0);
	
	outWorldPos = push.model * vec4(inPos, 1.0);
	outViewDirection = normalize(ubo.viewPos.xyz - outWorldPos.xyz);
}