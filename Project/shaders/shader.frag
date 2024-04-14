#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inCameraPos;


layout(location = 0) out vec4 outColor;

void main() 
{
	vec4 color = vec4(1, 0.8, 0.2, 1);
	const float ambient = 0.1;

	vec3 N = normalize(inNormal);
	vec3 L = normalize(vec3(0.2, 0.2, 1.0));
	vec3 V = normalize(inCameraPos);
	vec3 R = reflect(-L, N);
	vec3 diffuse = max(dot(N, L), ambient).rrr;
	float specular = pow(max(dot(R, V), 0.0), 64.0);
	outColor = vec4(diffuse * color.rgb + specular, color.a);
	
	outColor = pow(outColor,vec4(2));
}