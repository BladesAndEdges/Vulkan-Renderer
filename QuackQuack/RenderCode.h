#pragma once // This line serves the same purpose as the include guards. The aims to prevent this file to be included more than once in a single compilation.

#define GLFW_INCLUDE_VULKAN // If this preprocessor macro is defined, the included-below glfw header, will also include the relevant vulkan.h files, containing the Vulkan API's function definitions and implementations.
#include <glfw3.h> // Include the GLFW windowing API

#include <iostream>  // cout, cerr
#include <stdexcept> // runtime_error
#include <vector> // vector
#include <cstring> // strcmp (Used when we compare c-style strings)
#include <cstdlib> // Nothing really uses this?
#include <set> // set
#include <algorithm> // min, max
#include <fstream> // std::ifstream, is_open, tellg, seekg, read, close
#include<array> // array

/*GLM*/
#include<glm.hpp>

#include "Vertex.h"

/*Constants are usually good to be initialized as such, instead of hard-coded values, as we may reuse them in later stages*/
const int WIDTH = 800;
const int HEIGHT = 600;

//struct Vertex
//{
//	glm::vec2 pos;
//	glm::vec3 color;
//
//	/*Specifies the stride between values in the array*/
//	static VkVertexInputBindingDescription getBindingDescription()
//	{
//		VkVertexInputBindingDescription bindingDescription = {};
//		bindingDescription.binding = 0; // layout location basically
//		bindingDescription.stride = sizeof(Vertex); // Distance in bytes between one entry and another
//		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Move between entries per vertex
//
//
//		return bindingDescription;
//	}
//
//	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
//	{
//
//		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
//
//		/*Vertex position descruiption*/
//		attributeDescriptions[0].binding = 0; // From which binding the per vertex data comes from
//		attributeDescriptions[0].location = 0; // location directive of the input in the vertex shader
//		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // The type of the data, the amount of colour channels should match the number of components
//		attributeDescriptions[0].offset = offsetof(Vertex, pos); // Calculates offset from the original struct?
//
//		/*Textures position description*/
//		attributeDescriptions[1].binding = 0;
//		attributeDescriptions[1].location = 1;
//		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//		attributeDescriptions[1].offset = offsetof(Vertex, color);
//
//		return attributeDescriptions;
//	}
//};



/*Uniform Buffer OBject*/
struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;

	/*
		Data will be stored inside a buffer, and then accessed via that buffer
		in the vertex shader.
	*/
};

/*Data*/
//const std::vector<Vertex> vertices = {
//
//	{ { -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
//	{ { 0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f } },
//	{ { 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f } },
//	{ { -0.5f, 0.5f },{ 1.0f, 1.0f, 1.0f } }
//};

//const std::vector<Vertex> vertices = {
//
//	{ {-0.5f, -0.5f, 1.0f} },
//	{ {0.5f, -0.5f , 1.0f} },
//	{ {0.5f, 0.5f  , 1.0f} },
//	{ {-0.5f, 0.5f , 1.0f} }
//};
//
//const std::vector<uint16_t> indices = {
//	0, 1, 2, 2, 3, 0
//};

/*The struct which will query the device to return which families of queues are supported*/
struct QueueFamilyIndices
{
	/*Initial value of -1 represents "Not found" or "Not available"*/
	int graphicsFamily = -1;
	int presentFamily = -1;

	/*
	Once the neccessary queries to the device have been processed,
	we would either remain with -1 for one or either of the queues,
	or both would now have a positive index ( or 0).
	*/
	bool isComplete()
	{
		/*If this function returns true, then both type of queue famileis are supported on our device*/
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

/*Structure which carries information about our swapchain mechanism that would be platform specific*/
struct SwapChainSupportDetails // comment this
{

	VkSurfaceCapabilitiesKHR capabilities; // Swap chain capabilities. Dimensions of images. Maximum and minimum images that can be held within our swap chain. 
	std::vector<VkSurfaceFormatKHR> formats; // Format of the pixels in memory, and the type of color spaces that are supported (SRGB for example).
	std::vector<VkPresentModeKHR> presentModes; // A list of supported presentation modes. In a sense, this represents different methods, for presenting looping the images in the swap chain.
};

/*
A class that has been implemtented as a wrapper to embody
rendering a triangle to the screen using the Vulkan API.

OPINION WHY THE AUTHOR IMPLEMENTED THE TUTORIAL THIS WAY

1. Simplicity - The author wanted us to focus on the concepts of learning Vulkan. They were never concerned for good/bad software engineering patterns as this was never the goal of the tutorial to begin with.
2. Relates to Vulkan - Vulkan is built upon structures, that contain members, and we create them by declaring instances of these structures. Similarly, the author may have wanted to keep this convention by
using OOP style classes, members and methods. Similarly in the main function we would then simply have an instance of a wrapper class, which represents a small Vulkan program for rendering
a triangle on the screen.
3. Scalability - The class can be viewed as a template for a Vulkan program. I.e. the program that we have created for rendering a triangle is just a user case, our class could be called VulklanProgram and with
some (or a lot) of changes can construct any Vulkan program we would like.
4. Isolated - One key thing to notice is that the author has made all the members private, as well as the methods. This is a good design choice if we were to create a Physics engine to go along with our
HelloTriangleApp rendering engine, for example. This way the individual components would be kept separate of each other and we can build on top of this to have a larger Program class, which
could use both of the smaller HelloTriangleApp and Physics classes, if we like to.

*/
class RenderCode
{

private: // Private data, the authro most likely intended to keep rendering separate if the project was to develop in size, and have other components added to it.


	/**********************************************DATA********************************/

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	/*********************************************DATA*********************************/

	GLFWwindow* window; // The GLFW window object, which encapsulates two things: Both the window, and an OpenGL context ( By default)

						/*
						Vulkan functions:

						A quick note: Vulkan is written in C. As such, unlike c++ style namespaces, we still use
						the convention in C-like APIs to prefix functions with "vk", it's a way to know hat section
						of a larger codebase the given function belongs to.

						Vulkan Types:

						The Vulkan API objects are also prefixed with the word Vk. A useful way to think of them is
						as opaque handles. That's it. Their values don't need to be interpreted in any other way. It is just a handle
						which we pass to the functions as a neccessary paramater.
						*/


	VkInstance instance; // The Vulkan instance represents the connection between OUR application the Vulkan API.
	VkSurfaceKHR surface; // The surface is an interface between the NATIVE windowing API and the Vulkan API. Note, this is platform specific always.

	VkDebugReportCallbackEXT callback;

	VkPhysicalDevice physicalDevice; // This is a physical hardware device, capable of supporting and running Vulkan.
	VkDevice device; // This is an interface between our Vulkan Application and the physical device. 

	VkQueue graphicsQueue; // A set of commands that exectute draw calls
	VkQueue presentQueue; // A set of commands that execture presentation commands

	VkSwapchainKHR swapChain; // Represents, in a sense, an image looping mechanism. A description of a loop of how the different images produced will be output on the screen.

	VkFormat swapChainImageFormat;  // The chosen surface format will be stored in this variable. 
	VkExtent2D swapChainExtent; // A handle representing the resolution of the images inside the swap chain.

	std::vector<VkImage> swapChainImages; // A vector of image handles. The images conceptually are multidimensional arrays of data.

	VkRenderPass renderPass; // Denotes the number and type of formats used in the rendering pass.

	/*All bindings will be combined into a single descriptor set layout*/
	VkDescriptorSetLayout descriptorSetLayout;

	VkPipelineLayout pipelineLayout; // Configuration of the rendering pipeline in terms of what types of descriptor sets will be bound to the CommandBuffer	
	VkPipeline graphicsPipeline; // Defines our GRAPHICS pipeline(We can have different types of pipelines, compute one for example) and comprises of everything that was aforementioned. It is the largest object. 

	std::vector<VkImageView> swapChainImageViews; // In memory, our data is essentially bytes. Think of ImageViews as a way to only look at a specified range of these values and interpret them differently.

	std::vector<VkFramebuffer> swapChainFramebuffers; // An array of valid render targets which can be rendered to and then submitted to the Queue to execute on the device.

	VkCommandPool commandPool; // A simple object whose purpose is to allocate CommandBuffers. It is associated with a Queue Family. I.e. all command buffers submited would be of the same type.

	std::vector<VkCommandBuffer> commandBuffers; // A series of commands, that are of the same type, and are to be sent to a Queue. 

	 /*Semaphores are used to synchronize the application on a global level as it otherwise does not exist by default to ensure maximum performance. (Explained better in the cpp file)*/
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	VkBuffer vertexBuffer; // A handle referencing a vertex buffer;
	VkDeviceMemory vertexBufferMemory; // A handle to the vertexBuffer memory on the GPU

	VkBuffer indexBuffer; // Handle for the index buffer
	VkDeviceMemory indexBufferMemory; // a handle to the index buffer memory on the gpu

	VkBuffer uniformBuffer; // Also a handle, for the unform buffer. So are all handles just references?
	VkDeviceMemory uniformBufferMemory;  // Will need to allocated requested amounts of memory for its purpose.

	VkDescriptorPool descriptorPool; // The descriptor pool which contains the descriptor sets

	VkDescriptorSet descriptorSet; //The descriptor set that will contain all ... ubos? ResourceS? Something?! 


	/*Provides all the neccessary information Vulkan requires about our application*/
	void initVulkan();

	/*Main Rendering loop. Renders our frames on the screen, terminates once the window is closed*/
	void mainLoop();

	/*Once the Vulkan app is terminated, performs clean up and frees the used up resources*/
	void cleanup();

	/*Initializes the GLFW window and starts the Vulkan Context for rendering*/
	void initWindow();

	/*Sets up debugging information if validation layers were enabled*/
	void setupDebugCallback();

	/*Initializes the Vulkan library. It creates an "instance" representing the connections between the application and the Vulkan library*/
	void createInstance();

	/*Queries the device for a list of supported layers.*/
	bool checkValidationLayerSupport();

	/*Provides a list of supported Queue families and checks weather the Queue families we require are supported by the device*/
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	/*Returns a list of required extensions our Vulkan requries to be validated properly*/
	std::vector<const char*> getRequiredExtensions();

	/*Traverses the available devices and picks the device which has all the required Vulkan utilities*/
	/*The graphics card we will use will end up getting stored inside a handle*/
	void pickPhysicalDevice();

	/*Compares the required Vulkan extensions our applicaiton needs against those the device in question supports. Used to check if the device is capable of running our applicaiton*/
	bool isDeviceSuitable(VkPhysicalDevice device);

	/*Creates an interface between our physical device and our application. But first checks if all Queue family types are supported.*/
	void createLogicalDevice();

	/*Windowing systems are specific to it's native platform. As such we check the limits of the windowing api and see if it can support the different requirements of our application.*/
	void createSurface();

	/*Queries device for available extensions and checks if they are all supported*/
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	/*Queries and returns information about supported surface formats and presentation modes*/
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	/*Details about our format. Our format, if I am correct, is specifying details about the data used.*/
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	/*Could be tought of as supplying the information on whole swap chain images will be handled and passed to the window. I.e. if swap chain is the mechanism, then the presentation mode is the algorithm*/
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

	// A handle representing information about the resolution of the images within the swap chain
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	/*Creates a swap chain mechanism based on the information of surface formats, extent and presentation mode used*/
	void createSwapChain();

	/*Generating a set of paramaters which describe/referring to a specific image*/
	void createImageViews();

	/*Uses all the information from the other creation stages to finally produce the graphics pipeline capable of rendereing our application*/
	void createGraphicsPipeline();

	/*Reads a file and returns a buffer with it's contents */
	static std::vector<char> readFile(const std::string& filename);

	/*Creates a shader module which can be used in the graphics pipeline*/
	VkShaderModule createShaderModule(const std::vector<char>& code);

	/*Provides information about the amount and types of attachments and formats used*/
	void createRenderPass();

	/*A set of valid framebuffers that can be used as render targets*/
	void createFramebuffers();

	/*Generates/provides a sequence of instructions to be executed.*/
	void createCommandBuffers();

	/*Creats a larger set of command buffers, each of the same type, which have their own instructions.*/
	void createCommandPool();

	/*Ouputs a triangle to the screen, by aquiring the next image from the swap chain*/
	void drawFrame();

	/*Creates synchronization mechanisms for signaling different conditions for the image*/
	void createSemaphores();

	/*Recreates a swap chain whenever the system detects a resize event. Techincally recreates all the required stuff like framebuffers, image views etc. that depend on the swap chain*/
	void recreateSwapChain();

	/*Separates cleaning up the swap chain as we want older versions of framebuffers, image views, scissors, viewports etc. are fully destroyed before recreating them*/
	void cleanupSwapChain();

	/*Checks when recreation of a swap chain function is necccessary*/
	static void onWindowResized(GLFWwindow* window, int width, int height);

	/*Creates a vertex buffer and sets up the data, allocates memory etc.*/
	void createVertexBuffer();

	/*Finds the correct GPU memory type to use, based on information passed from our application and buffer requirements that got computed during creation*/
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	/*To create multiple buffers, as we`ll need a staging buffer and an actual buffer for the vertex buffer*/
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	/*Will be used to copy over data from the host-side staging buffer into the vertex buffer which is using device memory*/
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	/*Creates the index buffer, this is almost identical as the vertex buffer creation*/
	void createIndexBuffer();

	/*Describes the way the data is layed out in the unform buffer*/
	void createDescriptorSetLayout();

	/*Sets up the unofrm buffer information*/
	void createUniformBuffer();

	void updateUniformBuffer();

	/*Similarly to command buffers, we cannot access them directly, so descriptor sets are allocated from a pool*/
	void createDescriptorPool();

	void createDescriptorSet();

	/*
	VKAPI_ATTR and VKAPI_CALL ensure that the functions has the correct signature for the API to call it.


	The first three paramaters are the important ones.

	The first paramater is a flag, representing the type of error. I.e. a warning, an error, information, or perfromance warning, or debug report.
	The object type is the Vulkan object which triggered the error.
	The msg represents a human-friendly message stating what went wrong exactly.
	*/
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t locaiton, int32_t code, const char* layerPrefix, const char* msg, void* userData);

public:

	/*Runs the entire application. Sets up, main loop, clean up*/
	void run();

	/*
	The constructor and destructor. They serve no purpose here, but I decicded to keep them either way.
	*/
	RenderCode();
	RenderCode(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	~RenderCode();
};

