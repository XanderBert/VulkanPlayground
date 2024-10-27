#version 450
#include "PBR.glsl"

layout(push_constant) uniform constants
{
	mat4 model;
} push;

layout(set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 viewProjection;
	vec4 viewPos;
	vec4 cameraPlanes;

	vec4 lightPos;
	vec4 lightColor;
} ubo;


layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inTangent;

layout(location = 0) out vec4 outNormal;

void main()
{
	vec3 normal = normalize(inNormal);

	//Move the normal from [0, 1] to [-1, 1]
	normal = 0.5f * normal + 0.5f;

	//Output the normal
	outNormal = vec4(normal, 1.0f);
}