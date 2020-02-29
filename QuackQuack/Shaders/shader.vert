#version 450 // The version of glsl is important to be specified as usually there is a correspondance between the graphics api and the shading language version
#extension GL_ARB_separate_shader_objects : enable // Enables Vulkan shaders

/*This is vertex attributes*/
/*They are properties specified in the vertex buffer, per vertex*/
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() // A main function which is invoked for every vertex on
{

	gl_Position = vec4(inPosition, 0.0, 1.0); // 
	fragColor = inColor;
}