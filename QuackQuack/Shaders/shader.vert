#version 450

#extension GL_ARB_separate_shader_objects : enable // Enables Vulkan shaders

/*The order of the in, uniform and out does not matter*/
/*The binding directive is similar to the layout*/

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model; // model matrix
	mat4 view; // view matrix
	mat4 proj; // projection matrix
	vec3 worldViewPosition;

	float azimuth;
	float zenith;

	bool toggleTextures;
} ubo;

/*This is vertex attributes*/
/*They are properties specified in the vertex buffer, per vertex*/
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 worldVertexNormal;
layout(location = 1) out vec2 worldTextureCoordinate;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() // A main function which is invoked for every vertex on
{
	
	/*Matrix that takes us to world space*/
	mat3 modelMatrix3x3 = mat3(ubo.model);
	
	/*The normal in world space for the fragment*/
	worldVertexNormal = modelMatrix3x3 *  inNormal;

	/*The vertex position*/
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

	/*The texture coordinates*/
	worldTextureCoordinate = inTexCoord;
}