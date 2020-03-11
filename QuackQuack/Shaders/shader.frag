#version 450 // The version of glsl is important to be specified as usually there is a correspondance between the graphics api and the shading language version
#extension GL_ARB_separate_shader_objects : enable // Enables Vulkan shaders

//Indexes are specified to determine which variables between the in and out variables get linked together.
//Variables with the same index get linked by default

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model; // model matrix
	mat4 view; // view matrix
	mat4 proj; // projection matrix
	vec3 worldViewPosition; 
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 worldVertexNormal;// The normal vector of the vertex, expressed in world coordinates
layout(location = 1) in vec2 worldTextureCoordinate;


/*The fragment colour*/
layout(location = 0) out vec4 outColour;

void main()
{	
	/*Intensity values*/
	float ambientIntensity = 0.1;
	float specularIntensity = 0.5;

	/*Hardcoded values*/
	vec3 worldLightSourceVector = vec3(10.0, 0.0, 0.0); // The position of our light source in world coordinates

	/*Values from the mtl file for Ki*/
	vec3 Ka = vec3(1.0, 1.0, 1.0);
	vec3 Kd = vec3(1.0, 1.0, 1.0);
	vec3 Ks = vec3(0.2880, 0.2880, 0.2880);
	float lightSpecularExponent = 28.0;

	/*The colour of the material*/
	vec3 objectColour = vec3(1.0, 1.0, 1.0);

	/******AMBIENT******************************************************************************************************************/

	/*Ambient lighting is just light that is always present*/
	vec3 ambientLighting = ambientIntensity * (Ka * objectColour);

	/******AMBIENT******************************************************************************************************************/


	/******DIFFUSE******************************************************************************************************************/

	/*Normalize the vectors fo the dot product computation for diffuse*/
	vec3 normWorldLightSourceVector = normalize(worldLightSourceVector);
	vec3 normWorldVertexNormal = normalize(worldVertexNormal);

	/*The result from the dot product for diffuse computation*/
	float diffuseDotProduct = dot(normWorldLightSourceVector, normWorldVertexNormal);

	/*The diffuse lighting*/
	vec3 diffuseLighting = diffuseDotProduct * Kd * objectColour;
	
	/*****DIFFUSE******************************************************************************************************************/

	/*****SPECULAR******************************************************************************************************************/

	/*Normalized eye vector*/
	vec3 normWorldEyeVector = normalize(ubo.worldViewPosition);
	vec3 halfAngleVector = normalize( (normWorldEyeVector + normWorldLightSourceVector) / 2.0);
	float specularDotProduct = dot(halfAngleVector, normWorldVertexNormal);
	float specularPower = pow(specularDotProduct, lightSpecularExponent);
	vec3 specularLighting = specularIntensity * (Ks * (objectColour * specularPower));

	/*****SPECULAR******************************************************************************************************************/
	
	vec3 finalLightingColour = ambientLighting + diffuseLighting + specularLighting;

	outColour = texture(texSampler, worldTextureCoordinate);
}