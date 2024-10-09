#version 450

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

void main(){

}