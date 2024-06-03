#version 450
#include "PBR.glsl"

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

layout(set = 1, binding = 0) uniform uniformMaterial
{
	vec4 gamma;
	vec4 exposure;
} material;

layout(set = 1, binding = 1) uniform sampler2D albedoMap;
layout(set = 1, binding = 2) uniform sampler2D normalMap;
layout(set = 1, binding = 3) uniform sampler2D MetalRoughMap;
layout(set = 1, binding = 4) uniform samplerCube skyBox;


layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inTangent;

layout(location = 0) out vec4 outColor;

//struct Light
//{
//	vec3 position;
//	vec3 color;
//};


void main()
{
	vec3 N = calculateNormal(normalMap, inNormal, inTangent.xyz, inUV);
	vec3 V = normalize(ubo.viewPos.xyz - inWorldPos);
	vec3 R = reflect(-V, N);

	vec2 mr = texture(MetalRoughMap, inUV).rg;
	float metallic = mr.r;
	float roughness = mr.g;

	vec3 albedo = texture(albedoMap, inUV).rgb;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);


//	//Hard Define lights for now
//	const int numLights = 2;
//	Light lights[numLights];
//	lights[0].position = vec3(10.0, 0.0, 10.0);
//	lights[0].color = vec3(0.1, 1.0, 0.1);
//	lights[1].position = vec3(-100.0, 0.0, 0.0);
//	lights[1].color = vec3(1.0, 1.0, 0.0);

//	vec3 Lo = vec3(0.0);
//	for(int i = 0; i < lights.length(); i++)
//	{
//		vec3 L = normalize(lights[i].position - inWorldPos);
//		Lo += specularContribution(L, V, N, F0, metallic, roughness, inUV, albedo, lights[i].color);
//	}

	vec3 Lo = vec3(0.0);
	vec3 L = normalize(ubo.lightPos.xyz - inWorldPos);
	Lo += specularContribution(L, V, N, F0, metallic, roughness, inUV, albedo, ubo.lightColor.xyz);



	vec2 brdf = (0.08 * vec2(max(dot(N, V), 0.0), roughness)).rg;
	//vec3 reflection = prefilteredReflection(R, roughness).rgb;
	vec3 reflection = vec3(0.3);


	// Diffuse based on irradiance
	vec3 diffuse = 0.7 * albedo;

	vec3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);

	// Specular reflectance
	vec3 specular = reflection * (F * brdf.x + brdf.y);

	// Ambient part
	vec3 kD = 1.0 - F;
	kD *= 1.0 - metallic;

	mat4 inverseModel = inverse(mat4(push.model));
	vec4 skyboxReflection = SkyboxReflection(inWorldPos, inNormal, inverseModel, skyBox);
	vec3 ambient = skyboxReflection.rgb * 0.2 +  (kD * diffuse + specular);

	vec3 color = ambient + Lo;

	// Tone mapping
	color = ToneMap(color * material.exposure.xyz);
	color = color * (1.0f / ToneMap(vec3(11.2f)));
	// Gamma correction
	color = pow(color, vec3(1.0f / material.gamma.x));

	outColor = vec4(color, 1.0);

}