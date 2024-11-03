#version 450
layout(set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 viewProjection;
	vec4 viewPos;
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUV;



const vec2 vertices[3] = vec2[](
vec2(0, -3.0),  // Bottom-left
vec2( -3.0, 1.0),  // Bottom-right
vec2( 3.0,  1.0)   // Top-left
);

void main()
{
	// Set the vertex position in normalized device coordinates
	gl_Position = vec4(vertices[gl_VertexIndex], 0.0, 1.0);

	// Map vertex positions to texture coordinates (0,1) space
	//TexCoord = (positions[gl_VertexIndex] + vec2(1.0)) * 0.5;
}

