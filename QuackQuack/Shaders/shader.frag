#version 450 // The version of glsl is important to be specified as usually there is a correspondance between the graphics api and the shading language version
#extension GL_ARB_separate_shader_objects : enable // Enables Vulkan shaders

//Indexes are specified to determine which variables between the in and out variables get linked together.
//Variables with the same index get linked by default
layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(fragColor, 1.0); // Outputs the colour along with the colour value
}