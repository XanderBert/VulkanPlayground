#version 450

layout(push_constant) uniform constants
{
    mat4 model;
} PushConstants;

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 viewProjection;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outPostion;
layout(location = 1) out vec4 outWorldPosition;
layout(location = 2) out vec4 outCameraPosition;
layout(location = 3) out vec3 outNormal;

vec4 GetCameraPosition() 
{
    mat4 invertedViewProjectionMatrix = inverse(ubo.viewProjection);
    
    // Extract the camera position from the translation component of the inverted matrix
    vec4 cameraPosition = vec4(invertedViewProjectionMatrix[3]);
    
    return cameraPosition;
}


void main() 
{
	outPostion = ubo.viewProjection * PushConstants.model * vec4(inPosition, 1.0);
    outCameraPosition = GetCameraPosition();
    outWorldPosition = PushConstants.model * vec4(inPosition, 1.0);
	outNormal = inNormal;
	

	 gl_Position = outPostion;
}