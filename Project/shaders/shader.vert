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

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec4 outTangent;

void main() 
{
	vec3 localPosition = vec3(push.model * vec4(inPos, 1.0));
	outWorldPos = localPosition;

	outNormal = mat3(push.model) * inNormal;
	outTangent = vec4(mat3(push.model) * inTangent.xyz, 1);
	outUV = inUV;


	gl_Position =  ubo.viewProjection * vec4(outWorldPos, 1.0);
}