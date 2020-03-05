#version 450 // The version of glsl is important to be specified as usually there is a correspondance between the graphics api and the shading language version
#extension GL_ARB_separate_shader_objects : enable // Enables Vulkan shaders

/*The order of the in, uniform and out does not matter*/
/*The binding directive is similar to the layout*/

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model; // model matrix
	mat4 view; // view matrix
	mat4 proj; // projection matrix
} ubo;

/*This is vertex attributes*/
/*They are properties specified in the vertex buffer, per vertex*/
layout(location = 0) in vec3 inPosition;
//layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() // A main function which is invoked for every vertex on
{
	/*Update the position of each vertex every frame*/
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0); // Compute final positon in clip coordinates

	//fragColor = inColor;
}