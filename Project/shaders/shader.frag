#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inViewDirection;
layout (location = 3) in vec4 inWorldPos;

layout(location = 0) out vec4 outColor;

void main() 
{
	const vec4 color = vec4(1, 1, 0.8, 1);
	const float ambient = 0.1;
	const vec3 lightDirection = vec3(-0.2, -0.2, -1.0);

	//Diffuse
	float diffuseStrength = clamp(dot(inNormal, normalize(-lightDirection)), 1,0);
	float lambert = pow(diffuseStrength * 0.5 + 0.5, 2.0);
	//float lambert = diffuseStrength / 3.1415;
    
	//Calculate the halfvector
   	vec3 halfVector = normalize(inViewDirection + normalize(-lightDirection));
	float specularValue = clamp(dot(inNormal, halfVector),1 , 0);
	const float specularExp = 4;
	float specularPower = pow(specularValue, specularExp);
	
	
	outColor = color * lambert + specularPower ;
	//outColor = color;
	//Saturate
	outColor = pow(outColor,vec4(2.4));
}