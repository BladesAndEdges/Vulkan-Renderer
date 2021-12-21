#include "RenderCode.h" // Include all class deinitions of our user-defined class
#include "OBJReaderClass.h" // Include the obj reader

#include <glm.hpp>

/*The main entry point of our program*/
int main()
{
	/*The mesh data*/
	OBJReaderClass reader("Meshes/viking_room.obj");
	const std::vector<Vertex> vertices = reader.getVertices();
	const std::vector<uint32_t> indices = reader.getIndices();

	/*And instance representing the Vulkan application which renders a triangle to the screen*/
	RenderCode app(vertices, indices);

	/*A try block to enclose a function which could potentially throw an exception*/
	try
	{
		/*Execture all of our Vulkan private functions*/
		app.run();
	}
	/*Handle an exception which is triggered*/
	catch (const std::exception& e)
	{
		/*Output the exception to stdout*/
		std::cerr << e.what() << std::endl;

		/*Returns a implementation-specific error code */
		return EXIT_FAILURE; // The author has most likely used this to ensure people are not bound to a single system. I.e. the code is cross-platform.
	}

	/*
	Visual studio closes the console as soon as program has stopped executing. This caused me to use some hacks around the system. I could've used a breakpoint, but even that has it's issues.
	This would do for now.
	*/

	system("pause"); // A hack which launches the pause program which halts the execution of the program at hand. I use it so I can actually see the output in the command line window.
}

