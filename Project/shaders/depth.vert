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

void main()
{

    vec3 localPosition = vec3(push.model * vec4(inPos, 1.0));
    gl_Position =  ubo.viewProjection * vec4(localPosition, 1.0);
}