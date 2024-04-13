#version 450

layout(push_constant) uniform constants
{
    mat4 model;
} PushConstants;

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 view;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;

void main() 
{
    gl_Position = ubo.view * PushConstants.model * vec4(inPosition, 1.0);
    fragColor = inNormal;
}