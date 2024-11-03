#version 450

layout(push_constant) uniform constants {
	mat4 model;
} push;


layout(set = 1, binding = 0) uniform sampler2D albedoResource;
layout(set = 1, binding = 1) uniform sampler2D ssaoResource;
layout(set = 1, binding = 2) uniform uniformMaterial
{
	vec4 screenSize;
} material;

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 albedo = texture(albedoResource, gl_FragCoord.xy / material.screenSize.xy);
	vec4 ssao = texture(ssaoResource, gl_FragCoord.xy / material.screenSize.xy);

	//outColor = mix(albedo, ssao, 0.5);
	outColor = albedo * ssao;
}