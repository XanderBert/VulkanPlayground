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

//layout(set = 1, binding = 0) uniform uniformMaterial
//{
//	vec4 color;
//} material;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outViewDirection;


void main() 
{	
    //outNormal = material.color.xyz;
    outNormal = inNormal;
	outUV = inUV;
	
	vec4 pos = push.model * vec4(inPos, 1.0);
	gl_Position = vec4(pos.yx, 0.0, 1.0);
	
	vec4 worldPos = push.model * vec4(inPos, 1.0);
	outViewDirection = ubo.viewPos.xyz - worldPos.xyz;
}