#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inWorldPosition;
layout(location = 2) in vec4 inCameraPosition;
layout(location = 3) in vec3 inNormal;



layout(location = 0) out vec4 outColor;

vec3 GetDiffuse(vec3 normal, vec3 lightDirection)
{
	float diffuseStrength = clamp((dot(normal, normalize(-lightDirection))),0.0, 1.0);
	
	//Calculate the half lambert
	vec3 lambert = vec3(pow(diffuseStrength * 0.5 + 0.5, 8.0));

	//Calculate the diffuse color
    return lambert;
}

void main() 
{
    vec3 lightDirection = vec3(normalize(vec4(0.577, -0.577, 0.577, 1.0) * inWorldPosition));

	vec3 lamber = GetDiffuse(inNormal, lightDirection);
	vec4 color = vec4(lamber * inNormal, 1.0);

    outColor = color;
}