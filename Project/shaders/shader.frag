#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable
#extension GL_GOOGLE_include_directive : enable

#include "PBR.glsl"

layout(set = 1, binding = 0) uniform uniformMaterial
{
	vec4 color;
} material;

layout(set = 1, binding = 1) uniform sampler2D albedoMap;

layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inTangent;

layout(location = 0) out vec4 outColor;

void main() 
{

	outColor = texture(albedoMap, inUV);
	outColor.x = D_GGX(outColor.x, 1);
	outColor.xyz = ToneMap(outColor.xyz);
}