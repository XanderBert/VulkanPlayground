#version 450

//layout(set = 0, binding = 0) uniform sampler2D combinedTexture;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inViewDirection;

layout(location = 0) out vec4 outColor;

void main() 
{
	vec4 color = vec4(0.8, 0.8, 0.8, 1);
	const float ambient = 0.1;

	vec3 N = normalize(inNormal);
	vec3 L = normalize(vec3(0.2, 0.2, 1.0));
	vec3 V = normalize(inViewDirection);
	vec3 R = reflect(-L, N);
	vec3 diffuse = max(dot(N, L), ambient).rrr;
	float specular = pow(max(dot(R, V), 0.0), 64.0);
	outColor = vec4(diffuse * color.rgb + specular, color.a);
	
	//Saturate
	//outColor = pow(outColor,vec4(1.4));
	outColor = pow(outColor,vec4(1.4));
	//outColor = vec4(inNormal, 1.0);
}