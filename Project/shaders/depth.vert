#version 450

layout(push_constant) uniform constants {
    mat4 model;
} push;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 viewProjection;
    vec4 viewPos;
    vec4 lightPos;
    vec4 lightColor;
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUV;
layout(location = 3) out vec4 outTangent;

void main() {
    vec4 localPosition = push.model * vec4(inPos, 1.0);  // Local -> World space
    vec3 normalWorld = mat3(push.model) * inNormal;      // Local -> World space normal

    outWorldPos = localPosition.xyz;                     // World space position
    outUV = inUV;
    outNormal = normalWorld;

    gl_Position = ubo.viewProjection * localPosition;    // Transform to clip space
}
