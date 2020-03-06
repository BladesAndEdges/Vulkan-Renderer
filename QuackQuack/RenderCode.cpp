#include "RenderCode.h" // Include the definitions of the class.

#define GLM_FORCE_RADIANS // Make glm functions use radians
#include <glm.hpp>
#include <gtc/matrix_transform.hpp> // model transformations

#include <chrono>

/*
The logic behind this is that our Validation Layers are part of the system Vulkan uses
to debug. Similarly, debug mode is created with the core purpose of running tests to ensure
the program behaves as expected. In a sense, this limits Vulkan's own debugging within
a sandbox mode, outside of which it cannot affect the program.

Other than that debugging usually causes our programs to take a performance hit, as there
are additional conditions that need to be checked if they are valid.

Visual Studio also runs DEBUG mode with all optimizations switched off and release mode
with all optimizations switched on, for maximum performance.

*/
#ifdef NDEBUG // If the program is run in DEBUG mode.
const bool enableValidationLayers = false;
#else // If the program is run in release mode
const bool enableValidationLayers = true;
#endif 

const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation"}; // Contains the string name of the validation layer, provided by the creator of the SDK, which we need.

const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; // Similarly as above, we provide a list of the required extensions which we are looking for.

																					   /*
																					   Similarly to all vulkan objects. We must also supply the isntruction of freeing up the requested
																					   extension resource. This is up to us as programmers
																					   */
void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	/*As it is also part of the extension, we must load the the function via it's function pointer*/
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

	/*If the function was not present as a supplied extension*/
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}

/*
Vulkan is built on the basis of setting
up handles which describe different states
of a paramater neccessary for the rendering
of graphics.

We therefore have one large function
which initializes all those separate components
in the correct order it expects them.
*/
void RenderCode::initVulkan()
{
	createInstance(); // Vulkan link between api and application

	setupDebugCallback(); // User-defined function for custom errors we've chosen to not ignore form the validation layers

	createSurface(); // Sets up an interface object between the windowing API and the Vulkan application. Important to b called after the instance, rather before the physical device,  as it may influence the choice of teh grpahics card

	pickPhysicalDevice(); // Queries the machine for a list of valid hardware devices capable of running Vulkan

	createLogicalDevice(); // Provides a description of the requirements of our application that have to be handled by the physical device.

	createSwapChain(); // A mechanism for looping images that would be presented to the screen.

	createImageViews(); // Prepare information about which parts of the framebuffer will be read in and displayed in the final presentation stage.

	createRenderPass(); // Sets up the amount of formats and the type which are required.

	createDescriptorSetLayout(); // Should be called here as we will need it exactly after pipeline creation

	createGraphicsPipeline(); // Creates a graphics pipeline object with all information about extensions, swap chains, etc.

	createFramebuffers(); // Create a set of valid render targets

	createCommandPool(); // The command pool will accomodate a series of queues of the same type of operations. 

	createVertexBuffer(); // Sets up the vertex buffer 

	createIndexBuffer(); // Sets up the index buffer

	createUniformBuffer(); // Set up the uniform buffer

	createDescriptorPool(); // A descriptor pool is set up from which we will access descriptor sets

	createDescriptorSet(); // Creates our descriptor sets

	createCommandBuffers(); // Sets of instructions which are to be executed by a queue.

	createSemaphores(); // As Vulkan doesn't synchronize for us (Aims for maximum performance) we have to make set up our own synchronization if we deem it to be required
}

/*

This is the main rendering loop of our Vulkan renderer
It consolidates everything we've built up so far and
transforms it into a typical graphical application.

While running, it continously waits for any events
that may modify the contents of the window.

It contains a loop which is active until the window's
has been closed.
*/
void RenderCode::mainLoop()
{

	/*The function checks repeatedly at the start of the loop if glfw has been instructed to stop*/
	while (!glfwWindowShouldClose(window))
	{
		/*Checks continously for any changes that have been made and submits them immmedietely*/
		glfwPollEvents();

		updateUniformBuffer();

		/*Displays the triangle to the screen*/
		drawFrame();
	}

	/*This functions ensures all resources have been successfuly deallocated before continuing*/
	vkDeviceWaitIdle(device);
}

/*
Vulkan operates on giving as much power to the
programmer as possible. One of the capabilities this
encompasses is the ability for us to manage and set up
our own allocation systems. Custom allocators etc.

As a result, however, this means that we also manage
the destruction of these objects and must ensure all
resources are returned to the OS at the end of our
program's life cycle.
*/
void RenderCode::cleanup()
{

	/*
	Order of destruction matters in Vulkan, we start from the opposite side of how we created them.
	Or at least we try not to free a "parent resource" before all of it's "child" resources have been freed.
	*/

	cleanupSwapChain(); // Free swap chain resources

	vkDestroyDescriptorPool(device, descriptorPool, nullptr); // Destroys the pool of descriptor sets

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr); // The descriptor set should remain available up to the point we may need to create a new graphics pipeline. (End of the program)

	vkDestroyBuffer(device, uniformBuffer, nullptr); // The draw calls used by the unform buffer will be used until the end
	vkFreeMemory(device, uniformBufferMemory, nullptr); // So both memory and buffer should be used upon exitting the program

	vkDestroyBuffer(device, indexBuffer, nullptr); // Destroy the index buffer
	vkFreeMemory(device, indexBufferMemory, nullptr); // Free memory allocated to store the data from the index buffer 

	vkDestroyBuffer(device, vertexBuffer, nullptr); // The buffer should be available for during the entire rendering process, and only should be destroyed once we have no use for it anymore. I.e. when we terminate the program.
	vkFreeMemory(device, vertexBufferMemory, nullptr); // Free the memory allocated on the GPU for the vertexBuffer

	/*The semaphores should be cleand up at the end of the program once no mor synchronization is neccessary*/
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);

	vkDestroyCommandPool(device, commandPool, nullptr);

	vkDestroyDevice(device, nullptr); // Free the resources for the logical device interface
	DestroyDebugReportCallbackEXT(instance, callback, nullptr); // Free the resources for the debug function
	vkDestroySurfaceKHR(instance, surface, nullptr); // Free the resources for the surface handle. 
	vkDestroyInstance(instance, nullptr); // The Vulkan instance should be destroyed only upon exiting the application.

										  /*
										  This function will destroy the window and
										  it's context upon recieving a valid value
										  of GLFW's flag for closing a window
										  */
	glfwDestroyWindow(window);


	/*
	This function must be called before terminating
	the application. It frees all remianing windows, curosrs,
	and any other allocated resources.
	*/
	glfwTerminate();
}

/*
The Vulkan APU provides functions for rendering graphics to the computer screen.
The issue is that Vulkan can also be used for off-screen rendering, as such,
it does not really need to have a utility for showing images to the screen.

The authro therefore has used a windowing API known as GLFW. This has a few
issues that must be dealth with. As the name suggests, GLFW was created with
the intention to be a windowing API for OpenGL. As such, the default settings
for this was that the initialization functions ended up both creating a window
and an OpenGL context by default, as it was assumed that would be the preffered API.

Eventually, Vulkan came into existance, which meant that we want a Vulkan context, in
contrast to a OpenGL context. As such, we have to deal with this and provide GLFW with
information that we want a Vulkan context, and surpress the creation of a OpenGL context.
*/
void RenderCode::initWindow()
{


	/*Initializes the GLFW library*/
	glfwInit();

	/*GLFW was originally designed for OpenGL. We thereofre provide a hint to the library to not create the GL */
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	/*Provides a hint to the window to handle resize events*/
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	/*
	The first two paramaters are the window width and height. Prefferably, we would
	want these paramaters to NOT be hard coded as they may change at times.

	The thrid paramater will be displayed as the Window's title

	The fourth paramater allows us to choose between monitors. The specifeid monitor would
	be the location where the window will be rendered.(???)


	The fourth parameter is relevant only if we were using OpenGL.
	*/
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Coursework 1", nullptr, nullptr);

	glfwSetWindowUserPointer(window, this); // Set an arbitrary pointer to our window object that we can pass to functions that require it 
	glfwSetWindowSizeCallback(window, RenderCode::onWindowResized); // Used to specify a callback whenver a singal is issued for a resize event
}

/*
Our Debugging function is not part of the core api, it
is provided as an extension by the GPU vendor.

And we basically provide an implementation for it by specifying
what we require.

Therefore, we must load the adress of the function pointer.
*/
VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{

	/*Attempts to load the specified function pointer, if it's unsuccessful it would then return a nullptr;*/
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

	/*If the function was successfuly loaded, inform our application*/
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		/*Else extension was not present when we searched for it.*/
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}


/*
Enable validation layers if we run the application
only in debug mode.

In release mode, the additional checks would simply add overhead
to the performance of our application as they hook on
additional operations to each Vulkan function.

This function simply checks if the requested layers
are provided by the SDK.
*/
bool RenderCode::checkValidationLayerSupport()
{

	uint32_t layerCount;

	/*
	Return the amount of layers available by the SDK for our system.
	*/
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	/*Prepare a vector capable of storing the string names.*/
	std::vector<VkLayerProperties> availableLayers(layerCount);

	/*Store the names of the available validation layers inside our vector */
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	/*For every requested layer*/
	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		/*Check against every available layer*/
		for (const auto& layerProperties : availableLayers)
		{
			/*The string Id of a reuquested layer matches an available layer, choose that validation layer for usage (?)*/
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		/*If the layer has not been found in the list of available layers*/
		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

/*
Vulkan requires a debug callback extension to be enabled in order for us to recieve
readable data about what goes wrong from the validation layers.

The list of extensions is returend. This list represents lists of etensions rquired
by GLFW to run Vulkan. The list will always contain VK_KHR_surface.

Another thing. If we already know which extensions we need, we can pass them
manually in the list. This is what we do with the debug callback extension.

We make sure to only include that if we run in debug mode, i.e. only if we
need validation to be enabled in the first place.
*/
std::vector<const char*> RenderCode::getRequiredExtensions()
{

	uint32_t glfwExtensionsCount = 0;
	const char** glfwExtensions;

	/*Query for requried extensions by GLFW. This will be returned as string IDs*/
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

	/*Copies over that data into a list of extensions using... iterators??*/
	// glfwExtensions - pointer to beginnning, adding glfwExtensions will move the iterator to the last element (I think that is how it works).
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount); // vector constructor

																							   /*If run in debug mode*/
	if (enableValidationLayers)
	{
		/*USES THE OLDER VERSION, the newer tutorial uses VK_EXT_DEBUG_REPORT_EXTENSION_NAME */
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

/*
Retrieves a list of supported Queue families.

I.e. what operatiosn the queues can receive to submit to the device
*/
QueueFamilyIndices RenderCode::findQueueFamilies(VkPhysicalDevice device)
{

	/*
	vkQueueFamilyProperties contains information about the queue familiy.

	Supported operations and the number of queues that can be created based on that faimily.

	We care for the basic VK_QUEUE_GRAPHICS_BIT.
	*/

	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;

	/*Retrieve the number of queue family types that are supported by the device*/
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	/*An array capable of storing all the ids*/
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

	/*Retrieve their string ids*/
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;

	/*Iterate over all the queue families retrieved in the ist*/
	for (const auto& queueFamily : queueFamilies)
	{
		/*If the family supports at least one queue, and if the core grpahics queue is one of them*/
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			/*Set the value of our struct to i, which would always be postive value, meaning it is a valid queue.*/
			/*remember queues which are invalid were given a negative value*/
			indices.graphicsFamily = i;

			/*Although the windowing api may support presentation, some device operate only for compute operations.*/
			/*Furthermore, the graphics family may or may not overlap the features required for presentation, as such we must check them separately*/
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			/*If a presentation family queue is found*/
			if (queueFamily.queueCount > 0 && presentSupport) {
				indices.presentFamily = i;
			}
		}


		/*If the queue passed the previous if statement, it would contain a psotiive value, meaning we can break out of it???? The increment i??? */
		if (indices.isComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

/*
Combines queries for all features we require for the
successful run of our program by the physical device.
*/
bool RenderCode::isDeviceSuitable(VkPhysicalDevice device)
{
	/*Check if all the queue families are available, we mainly want the graphics family at this point*/
	QueueFamilyIndices indices = findQueueFamilies(device);

	/*Check if all extensions are supported for debugging or other external features not provided by the core api*/
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	/*Check if we are able to support swap chains. Not all devices are designed for graphical output*/
	bool swapChainAdequate = false;

	/*If the swap chain is supported retrieve the neccessary details*/
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	/*
	To determine if the device is suitable we need to have

	a) The required queue families for rendering an image and then presenting it onto a screen
	b) Extensions to utilize Vulkan utilities, such as swap chains and others
	c) The last checks if the swap chain is valid by checking all of it's own dependencies, such as surface formats, presentation moodes etc.

	Assuming all of these values return true, then the device is suitable to run our program.
	*/
	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

/*
Passes the information of our debug callback function.
Which validation layer is being used. The Type of errors
we want to worry about. IT is fully customizable to
only include errors which we worry about.
*/
void RenderCode::setupDebugCallback()
{
	/*If the validation layers have not been enabled*/
	if (!enableValidationLayers)
	{
		return;
	}

	/*Similarly we pass in a structure with details about our applicaiton*/
	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT; // The type of object we're creating
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT; // Filters the entirety of messages by only receiving the ones we want (the rest are ignored)
	createInfo.pfnCallback = debugCallback; // Specifies the pointer to the callback function 

											/*Attempt to create the debug function object*/
	if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to set up debug callback!");
	}

}

/*
This function shold be the first thing called. It creates
the link between the application and the Vulkan library.
It involves specifying some details about our applicaiton
to the driver. The reason behind this is that the driver
can then increase performance by including specialized
optimizations for our application.
*/
void RenderCode::createInstance()
{

	/*Check if validation layers are available by the SDK in use and we have found a suitable validation layer for our applicaiton*/
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("Validation layers requested, but not supported");
	}

	/*A struct providing information to the Vulkan library with information about our application*/
	VkApplicationInfo appInfo = {};

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // The type of strucure we are passing information of.
	appInfo.pApplicationName = "Vulkan Coursework 1"; // The title of our application.
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // The API version of Vulkan we are targetting
	appInfo.pEngineName = "No Engine"; // Indicates we have not used a specific engine for the creation of our application. May have just been an nullptr to indicicate that.
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); // The version number of the engine used to create the application
	appInfo.apiVersion = VK_API_VERSION_1_0; // This most likely is designed for forward compatibility. It specifies a maximum API version that our application can be used on. 

											 /*Tells vulkan which global extensions we wish to use and validation layers as well*/
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo; // Helps the implementation recognize behavoiour which is inherent to classes of applications (??)

											/*Get the rquired extensions from glfw for running our application*/
	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size()); // The amount of extensions required
	createInfo.ppEnabledExtensionNames = extensions.data(); // Pointer to the address where the list of extensions is stored

															/*
															If the validation layers are available with the SDK
															and we are running in debug mode.
															*/
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()); // Pass in the amount of the validation layers we will be using. We have only one. The LunarG SDK one.
		createInfo.ppEnabledLayerNames = validationLayers.data(); // Pass in the string representation of the layer. It is a pointer to the first element in our case.
	}
	else
	{
		/*Do not use validation layers, as they are not enabled*/
		createInfo.enabledExtensionCount = 0;
	}

	/*
	Attempt to create a vulkan instance. And store it in the handle we have as a class member.
	*/
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Vulkan instance!");
	}
}

/*
A lot of the funcitonality that Vulkan provides is
hardware-dependent. We therefore must request information
about the device to check if:

a) Vulkan is supported in general
b) Required extensions are available
c) General information about hardware properties (Memory etc.)

To be it simply: We need a suitable graphics card which support the features of our application.

Some machines use more than a single device. This is also important. We must specify which one of them we'd like to use, if both were deemed suitable.
*/
void RenderCode::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;

	/*Count the amount of devices there are on the machine we are working one, which support Vulkan*/
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	/*Aborth the application if none of them support the Vulkan*/
	if (deviceCount == 0)
	{
		throw std::runtime_error("Failed to find GPU with Vulkan support!");
	}

	/*Ohterwise create an array that would store the devices of size equal to that of the suitable amount of hardware*/
	std::vector<VkPhysicalDevice> devices(deviceCount);

	/*Recieve their Ids and store them inside the array as strings*/
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	/*Traverse the list and check is suitable for our requirements. i.e. if it supports all required features*/
	for (const auto& device : devices)
	{
		/*Since we do not care about performance or other metrics. The first device that is suitable should do*/
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			break;
		}
	}

	/*If no device was found, throw an error*/
	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to find a suitable GPU");
	}
}

/*
The phycial device represents a handle with
information about the hardware capabilities.

The logical device is an interface between
the graphics card and our application. It is
a structure which contains information
about what our application wants to use from
these now available features which are
stored in the physical device handle.
*/
void RenderCode::createLogicalDevice()
{
	/*Get the available queue families*/
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector <VkDeviceQueueCreateInfo> queueCreateInfos;

	/*The unique queue families our application requires for it's use*/
	std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

	/*Vulkan allows you to prioritize scheduling by giving a priority float between 0-1 on the importance of a queue*/
	/*This is neccessary even if you only have a single queue*/
	float queuePriority = 1.0f;

	/*The important thing of this loop is queueCount. The amount of queues each family would be given*/
	for (int queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority; // Set the same priority for each queue
		queueCreateInfos.push_back(queueCreateInfo);
	}

	/*We now have all the information supplied as what our application requires to run. We beging creating a logical device*/
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	/*Pass the requirements of each type of Queue family*/
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	/*Pass knowledge of the required capabilities of the device*/
	createInfo.pEnabledFeatures = &deviceFeatures;

	/*Enable all of the required etensions*/
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	/*We enable the same validation layers as we did for oru Vulkan isntance*/
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	/*If run in release mode*/
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	/*Attempt to create the logical device interface*/
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a logical device");
	}

	/*We retrieve handles for each queue family as Vulkan does not provide an otherwise implicit method to interface with them later on*/
	/*If the families overlap, they would have the same handle value*/

	vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue); // A handle to the graphics queue family.
	vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue); // A handle to the presentation queue family
}

/*
Behind the scenes retrieves information by recieving a
handle ot a window instance and the currently runnign process.

The glfwCreateWindowSurface does exactly that, with a different
implementation for each platform.
*/
void RenderCode::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface!");
	}
}

/*
Not all devices support every extension our application may require.
Vulkan provides us with functions that can query these lists and then
we can performa  loook up through this list weather or not the extensions
we require aare supported by the device we use.
*/
bool RenderCode::checkDeviceExtensionSupport(VkPhysicalDevice device) {

	/*To allocate an array, we need to know the amount of elements we want to store*/
	uint32_t extensionCount;

	/*We therefore query the device to iterate through the list and store the amount of extensions available*/
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	/*Now we can allocate an exact amount to allocate the details of our extensions.*/
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);

	/*Iterate and store the names and version of the available extensions.*/
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	/*Copy over all the required extensions into this vector*/
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	/*If the string names of our required extensions are found, erase them from the array*/
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	/*The above for loop would remove extensions that have succesfully been found. If all extensions were found, the device meets the requirements and would therefore be left with an empty array*/
	return requiredExtensions.empty();
}

/*
If the swap chain is available as an offered extension,
this does not mean that it can work for our applicaiton.

We need to query information in order to confirm that it
is not onlyu compatible with the isntance and the device,
but additionally for things like the VkSurface.
*/
SwapChainSupportDetails RenderCode::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	/*Checks the surface and the device to see the supported capabilities for the swap chain creation*/
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	/*Queries the supported surface formats. Surface formats are mainly the way data is presented*/
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	/*Queries the supported presentation modes for the swap chain mechanism. I.e. the "algorithm" for swapping over images*/
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

/*
If a valid swap chain is found, we now must specify the required swap chain properties
we would like to run for our program.

This is important as each program may have an ideal value in mind for the application,
but must query if that property is supported by the swap chain instance we were provided with.
*/
VkSurfaceFormatKHR RenderCode::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{

	// Choices of format is important as it represents the the way data is stored. I.e. the amount of channels, the memory requirements for each channel etc.
	// Tests if SRGB is available to use or not

	/*
	If Vulkan has no preffered surface format.
	*/
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		// Choose the first one which we want
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	//If however, we are not allowed to choose our own format, we would traverse the list and see a possible combination
	for (const auto& availableFormat : availableFormats)
	{
		// Try and search for our required combination
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	//If this also fails, then we can just settle with the first option provided
	// Take note that we could in fact rank the combinations to find a secondary best, but for the purpose of the tutorial that seems extensive.
	return availableFormats[0];
}

/*
This setting handles the way we would present our iamges to the screen.
*/
VkPresentModeKHR RenderCode::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	//Our preffered option
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR; // Implemented as a queue. The swap chain presents the first image in that queue, while loading a rendered image in queue at the back. This mode is guarranteed to be available.

	for (const auto& availablePresentMode : availablePresentModes)
	{

		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) // Good for performance. Non-bloocking. Whenever the queu becomes full, it should by convention block incoming images. This, instead, updates the queue with the newer images.
		{
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) // Images would be flushed to the screen immediately upon submission. May result in tearing.
		{
			bestMode = availablePresentMode;
		}
	}

	return bestMode; // Since the FIFO_KHR mode is guarranteed to be available, if none of our ideal modes are available, we settle with this one.
}

/*
The swap extennt represents the resolution which best matches window configuraiton
we have.

This value would suually always match the screen width and height, but isnot required to.
It may differ. Therefore we can provide int the maxium value of uint32_t if we wish that to be the case.
*/
VkExtent2D RenderCode::chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities)
{
	/*If we are not required to match the window resolution, simply return the current width and height of the sruface*/
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	/*Else make the surface match the resolution of our window*/
	else
	{
		/*Our window dimensions*/
		int width, height;

		/*Query glfw to return these values and store them in width and height*/
		glfwGetWindowSize(window, &width, &height);

		/*Set the extent to whatever the window resolution values are*/
		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		/*Our actual extent that would be made our current extent would end up choosing a value between the minimum and maximum extent values.*/
		/*That extent would end up matching the window resolution exactly if it is within the range, otherwise it would get clamped*/
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

/*
Creates the swap chain mechanism using the
help of the choose***** functions above

The swap chain represents the mechanism
with which images are looped over one
by one and presented to the screen.
*/
void RenderCode::createSwapChain()
{
	/*Retirieve swap chain requirements*/
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	/*Choose our specified swap chain properties*/
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	/*The amount of images that can be used in our swap chain "queue". */
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1; // Try to have the minimum amount + 1. The minimum amount would usualy be a single image (1) to implement double-buffering we would therefore required 2.

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount; // maxImageCount with a value of 0 woudl only indicate that we are bound only by memory to the amount of images in the queue.
	}

	VkSwapchainCreateInfoKHR createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount; // Amount of images in the queue
	createInfo.imageFormat = surfaceFormat.format; // The data format we have used
	createInfo.imageColorSpace = surfaceFormat.colorSpace; // The colours space used, i.e. linear or nonlinear etc.
	createInfo.imageExtent = extent; // The resolution of the images in the swap chain
	createInfo.imageArrayLayers = 1; // Amouint of layers each image consists of. This value is always 1, except in some cases of 3D applications.
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Specify the type of the render target

																 /*Next we have to specify how the separate unique queue families will handle the images.*/
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily }; // We only support/require the graphics and the presentation family

																										   /*If the families do not overlap (it's not a signel queue family)*/
	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // Images end up being used amongst multiple queue families with no explicit owenership transfer
		createInfo.queueFamilyIndexCount = 2; // there are 2 unique families
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Ownership of an image belongs to a single queue and must be explicitly transfered
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // If we want a transformation to be applied to the images within a swap chain. May be usefyl for trippy games when you play a bit drunk. :D
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Used to specify if the alpha channel should be used for blending, the specifed one here means we ignore it. 

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE; // Allow clipping basically. Ignore pixels that are obscured

	createInfo.oldSwapchain = VK_NULL_HANDLE; // Upon changes applied to the swap chain, like resizes. We may end up having to need to keep a reference to te old swap chain which is now either invalid or unoptimized. This may happen during runtime.

											  /*create the swap chain with the info we hae provided*/
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain!");
	}

	/*We can now retrieve the image handles within our swap chain queues*/
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr); // The implementation may have allowed more images to be rendered, thus the value myst explicitly be passed as a paramater once more
	swapChainImages.resize(imageCount); // If the value has changed, resize the amount of images contained with the new size gotten at runtime.
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data()); // Get the handles for the images

																					 /*Store the format and the extent of our swap chain for future use*/
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

/*
Creates an image view object.
An image view represents exactly that - a view into the image
inside the swap chain.

It describes how to access and where to access the VkIamge in
the swap chain.

IF we understand the data as simply
being a 2d texture array stored in memory, we can then
query a particular part of that array to view. This
may or may not be of the same dimensions as the image itself,
and focus only on a part of it.
*/
void RenderCode::createImageViews()
{
	/*Resize the list with all image views that would be created*/
	swapChainImageViews.resize(swapChainImages.size());

	/*Loop over these images*/
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i]; // Create a vkImage
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Treat the images as 2d textures
		createInfo.format = swapChainImageFormat; // Treat the image data as similar to the format we have in the swap chain

												  /*This is the only thign I do not honestly understand*/
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // How the image should be treated. In our case it's a normal image, RGBA etc. 
		createInfo.subresourceRange.baseMipLevel = 0; // The first mipmap level accessible to our view 
		createInfo.subresourceRange.levelCount = 1;  // The amount of mipmap levels. We only have 0, so this should be set to 1.
		createInfo.subresourceRange.baseArrayLayer = 0; // The first array layer accessible ot the view
		createInfo.subresourceRange.layerCount = 1; // The number of array layers. Can it be used for VR by switching between right and left eyes?

													/*Create the imageViews*/
		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image views");
		}
	}
}

/*

The graphics pipeline object represents
the operations required for us to take
our vertices and meshes to produce an image on
the screen with pixels.

1. The application first requires raw vertex data to be taken by the input assembler and collected.
2. The vertex shader which is one of the programmable stages takes the vertices as input and applies transformations to them. Takes them from model space to screen space.
3. An optional tesselation shader can be included ( not here) which can subdivide our geometry with the goal of increasing surface details.
4. Geometry shader, similar to the tesselation shader, works on primitives (Lines, points, triangles, triangle strips) and can output more than the inital input amount.
5. Teh rasterzation stage performs a series of operations. It interpolates attributes across the fragments and also performs depth testing to discard fragments that are behind other fragments.
6. The fragment shader will then be invoked on fragments which survived. It decides in which framebuffer each fragment will be written to and with what values for colour and depth.
7. If we have transluscent or transparent objects, we also have a colour blending stage to determine how to treat the colour of fragments who happen to map to the same cell in the framebuffer.

*/
void RenderCode::createGraphicsPipeline()
{

	/*
	Vulkan uses bytecode in oposed to GLSL human readable syntax
	reasons for this is the the compiler is less complex and
	human-readable syntax had issues when running on machines
	with different device properties.

	Code compiled on one machine with a certain device,  may not neccessarily execute
	on a machine with a different device,

	Khronos provides a compiler which translates glsl code into
	SPIR-V bytecode.
	*/

	/* Load the translated glsl bytecode form the vertex and framgent shader*/
	auto vertShaderCode = readFile("Shaders/vert.spv");
	auto fragShaderCode = readFile("Shaders/frag.spv");

	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;

	/*Loads the shader modules*/
	vertShaderModule = createShaderModule(vertShaderCode);
	fragShaderModule = createShaderModule(fragShaderCode);

	/*So far our shader modules are just wrapper around the bytecode*/
	/*We therefore specify a stage in the pipeline which will use that data*/

	/*Pass the vertex shader module to the vertex shader*/
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main"; // Since we may have multiple shaders in the shader module (?) we have to specify the entry point which will be unique for each shader inside the module. This way we distinguish them.

										/*Pass the fragment shader module to the fragment shader*/
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	/*This array will be used to reference the stages later on*/
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	/*Gets the binding descriptions which we have created. It recieves information about the layout of the bindings ( if there are more than one) and the layout of the attributes contained in the bound array*/
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	/*
	Description of the format of the vertex data

	Describes the following:

	Bindings: spacing between data and whether the data is per-vertex or per-instance (see instancing)
	Attribute descriptions: type of the attributes passed to the vertex shader, which binding to load them from and at which offset
	*/
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO; // Specify it is per vertex instead of per instance
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Most likely passing information of the binding to be used, such as the index in teh array, the elements it is build from(Vertex) and if it is per vertex or per instance
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()); // The amount of attributes in a single instance ( Vertex, in our case)
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	/*
	Describes the type of geometry which will be drawn
	and if primitve restart should be enabled.
	*/
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Triangle from every 3 vertices without reuse
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	/*
	The viewport is the region of the framebuffer we
	will be rendering to. This would most often
	be the entirety of our screen.
	*/
	VkViewport viewport = {};
	viewport.x = 0.0f; // From the top left corner.
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width; // The width we have for our images in the swap chain. Done to match the window resolutions.
	viewport.height = (float)swapChainExtent.height; // The height we have for our images in the swap chain
	viewport.minDepth = 0.0f; // The min depth and max depth should stick to 0,0 and 1,0 if we are not doing anything that requires depth buffering.
	viewport.maxDepth = 1.0f;

	/*The scissor recatangle defines which pixels of the image will be stoed in the framebuffer*/
	VkRect2D scissor = {};

	/*In this case we want one which renders the entire framebuffer so we specify no offsets*/
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	/*Combines the viewport and scissor rectangle into a viewport state*/
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	/*
	The rasterizer will take all of the non-clipped vertices
	and transform them into valid pixels, i.e. fragments.
	*/
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE; // Fragments beyond the near and far plane will be discarded instead of clamped

	rasterizer.rasterizerDiscardEnable = VK_FALSE; // If set to true disables any output to the the framebuffer 

	rasterizer.polygonMode = VK_POLYGON_MODE_LINE; // Determines how fragments are generated for geometry
	rasterizer.lineWidth = 1.0f;

	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; //Type of face culling used
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // The order direction for vertices to be ackoledged as front-facing 

													//Usefull for shadow mapping. Can be used for shadow mapping to bias them ( Removes shadow acne).
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	/*
	Configure multisampling. Basically it's used to sample multiple polygons
	would rasterize to same pixel. It is used to fix artifacts along the edges
	where the boundary seems like a rought transition rather than a smooth
	one. Known as antialiasing.
	*/
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	/*
	Usually cells in the framebuffer would already have a
	value previously stored in them. It is therefore important
	to set up a concention on whole we wish to do update the cell
	with the new value.

	A couple of options exist.
	Mix the old value with the new one.
	Combine them using a bitwise operation
	*/
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	/*
	Specifies a structure specifying parameters of a newly created pipeline color blend state
	*/
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	//Maybe mention that some parts of the pipeline here can actually be changed dynamically??

	/*
	You can use uniform values in shaders, which are globals similar to dynamic state variables that can be changed
	at drawing time to alter the behavior of your shaders without having to recreate them.
	They are commonly used to pass the transformation matrix to the vertex shader,
	or to create texture samplers in the fragment shader.
	*/
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1; // This specifies the amount of descriptor layouts the pipeline will make use of. 
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = 0;

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout!");
	}

	/*
	The graphics pipeline now combines all information
	about shader stages, fixed-function state,
	pipeline layout for uniform data and push constants,
	render pass information.
	*/
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;

	pipelineInfo.layout = pipelineLayout;

	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0; // Specifies the subpass index where the graphics pipeline will be used

							  /*Optimization step from Vulkan, pipeline derivatives are pipelines derived by some base pipeline*/
							  /*Saves time as it would have most of it's functionality to be similar and just copies it in*/
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	/*The VK_NULL_HANDLE is for a pipeline cache*/
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline!");
	}

	/*Once the data has been passed along the graphics pipeline, we don't really require the buffers anymore hence free their memory*/
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

/*
This function loads binary data from a file.
This is used to load the SPIR-V bytecode that
was produced when we compiled our shaders.
*/
std::vector<char> RenderCode::readFile(const std::string & filename) // Pass in the file path
{
	/*Read the file as binary data, and begin reading it from the back*/
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	/*If a link to the file has not been successfully associated with as stream*/
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file!");
	}

	/*The returned position at the end of the file and be used to set the file size*/
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize); // And a buffer capable of holding the binary data

	file.seekg(0); // Extracts the beginning of the file
	file.read(buffer.data(), fileSize); // Read in filesize bytes from position pointed to by buffer.data() This would point to the start of the file

	file.close(); // Close the file for reading

	return buffer;
}

/*
This function takes the bytecode produced in SPIR-V format and
creates shader module object from it.

A shader module can be a collection of shader code, with different
entry points (we use main here) with which we identify them by.
*/
VkShaderModule RenderCode::createShaderModule(const std::vector<char>& code) // Pass in the bytecode
{
	VkShaderModuleCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size(); // Size in bytes
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); //Pointer to the data

																	   /*Create the vk shader module*/
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Shader Module!");
	}

	//The buffer witht he code can be free immediately after this, like in OpenGL;

	return shaderModule;
}

/*
We need to specify how many color and depth buffers there will be, how many
samples to use for each of them and how their contents should be handled throughout the rendering operations.
*/
void RenderCode::createRenderPass()
{
	/*A single coloru buffer attachmetn represented bv animage in teh swap chain*/
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat; // Format should match that of the swap chain
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Stick with one sample as we have no multisampling

													 /*Defines how to handle the data before and fter rendering*/
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear values to a constant before rendering
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // STore contents in memory for rendering later

															/*We have are not usign stencil calculations, so existing contents are undefined and we do not care about them*/
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	/*Describes the memory layout of the data before and after rendering.*/
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	/*Each subpass references an attachment that we have specified*/
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	/*
	A single render pass can consist of multiple subpasses.Subpasses are subsequent rendering operations that depend on the contents
	of framebuffers in previous passes, for example a sequence of post - processing
	effects that are applied one after another.*/
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	/*
	The main render pass consists of a reference
	to the attachmetns the subpasses will use
	and the subpasses themselves, which depend
	on it.
	*/
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

	/*Vulkan contains implicit "subpasses". These are the operations right before and right after a render pass*/
	/*We therefore must handle suynchronization here as well.*/
	VkSubpassDependency dependency = {};

	/*Specifies the index of the dependency and the dependent subpass*/
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	/*
	The next two fields specify the operations to wait on and the stages in which these operations occur.
	We need to wait for the swap chain to finish reading from the image before we can access it.
	This can be accomplished by waiting on the color attachment output stage itself.
	*/
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // The stage at which this synchronization must occur is in the color attachment stage
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // And it involves writing and reading the colour information

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
}

/*
A framebuffer is a set of valid
VkImage that we use using the framebuffer.
*/
void RenderCode::createFramebuffers()
{
	/*Resize the buffer to accomodate all framebuffers*/
	swapChainFramebuffers.resize(swapChainImageViews.size());

	/*Create framebuffer for each image view*/
	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		VkImageView attachments[] = { swapChainImageViews[i] };

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass; // Render pass our framebuffer must be compatible with
		framebufferInfo.attachmentCount = 1; // We only pass the colour attachment
		framebufferInfo.pAttachments = attachments;

		/*Framebuffers should have the same resoltuions as window image width and height*/
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}

/*
Individaul draw calls cannot be accessed or used directly.
Instead, Vulkan requires them to be placed inside command
buffers.

The advantage of this is that all of the hard work of setting up the drawing commands can be done in advance and in multiple threads.
After that, you just have to tell Vulkan to execute the commands in the main loop.

*/
void RenderCode::createCommandPool()
{
	/*Command pools will have commandbuffers of the same type only, so we must have information about which type of queues are supported*/
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);


	/*Command buffers are executed by submitting them on one of the device queues, like the graphics and presentation queues we retrieved.
	Each command pool can only allocate command buffers that are submitted on a single type of queue.
	We're going to record commands for drawing, which is why we've chosen the graphics queue family.*/

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily; // draw commands
	poolInfo.flags = 0; // Hints how new commands are being recoreded. If they often change or they persist.

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool!");
	}
}

/*
Command buffers are used to record operations
that we would like to perform.

We only want to perform graphics operations(drawcalls)
here. Some draw calls, however, require binding the
correct framebuffer, as such we must record a command buffer
for every image inside our swap chain.
*/
void RenderCode::createCommandBuffers()
{
	commandBuffers.resize(swapChainFramebuffers.size());

	/*Struct that would be passed to the Vulkan allocation function*/
	VkCommandBufferAllocateInfo allocInfo = {};

	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Specify weather the allocated command buffers are primary or secondary.
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffer!");
	}

	/*Begin recording command buffers*/
	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // How we're going to use the command buffer. Can be resubmitted while pending execution, in this case.
		beginInfo.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

		/*Bind the correct framebuffer for each image, and reuse the same renderpass as we only have one we're interested in*/
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[i];

		/*Keep the rendering area to the same dimensions as the whole window*/
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		/*When the framebuffer is reset, update the values to black*/
		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // Execute the command buffers with only the primary command buffer itself is provided and no secondary command buffers are there.

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline); // Bind the GRAPHICS pipeline

																								 /*
																								 The number are as follows

																								 3 - vertices in the triangle
																								 1 - Triangle in the scene
																								 0 - Offset is 0 as data is tightly packed
																								 0 - Offset between instances is 0, we only have one
																								 */

		VkBuffer vertexBuffers[] = { vertexBuffer }; // We only have one vertex buffer

		VkDeviceSize offsets[] = { 0 }; // This array specifies a one-to-one mapping between the ammount of vertex buffers and the offsets of each buffer, i.e from where to start reading vertex data from.

		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets); // This call is used to bind vertex buffers to bindings.

		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32); // You can only have one idnex buffer, apparently

		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr); // They are not unique to graphics pipelines. Hence we specify the bind point to be graphics, 

		//vkCmdDraw(commandBuffers[i], 3, 1, 0, 0); /**DRAW THE TRIANGLE***/

		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffers[i]); // End render pass

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer!");
		}
	}
}

/*

This function will put everything together,

It will be called from the main loop and
would output the image to the screen.

This function will acquire an image from the swap chain,
execute the command buffer with that image as attachment in the framebuffer
and return the image to the swap chain for presentation.

Most of the functions, howver, are excuted asynchronously.
Vulkan doesn't provide any default synchronizaiton and as such we are therefore
meant to synchronize it ourselves
*/
void RenderCode::drawFrame()
{

	/*Acquire an image that is ready to be rendered from the swap chain via it's index*/
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex); // Specify the device and swapchain from which to acquire the image. Time in nanoseconds is passed for the image to be made available.

																																							/*If the swap chain has become incompativle with the surface*/
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}
	/*If the surface is still valid, but it's properties are not*/
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	/*Queue submission and synchronization to the device*/
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore }; // The semaphores which must be waited on
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // The stages of the pipeline when these semaphores will be waited on. This is the stage of the pipeline which is used for writing to the colour attachment
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	/*Submit the command buffer that binds the swap chain image*/
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	/*Specifies which semaphores to signal once command buffers have finished execution*/
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	/*This submits the queue to the device for execution*/
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	/*
	Once we have set up all the requirements to produce a complete image,
	we must submit it to the swap chain for presentation.
	*/
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores; // Wait for these semaphores before a presentation can happen

													/*The swapchain which will be used*/
	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex; // Index of the image for each swap chain

	result = vkQueuePresentKHR(presentQueue, &presentInfo); // Submits the request to the swap chain to show an image on the screen.

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	vkQueueWaitIdle(presentQueue); // Wait for presentation to fully finish before drawin the next frame

}

/*
Creates two semaphores for global syncrhonization.
One semaphore to signal that an image has been acquired and can be rendered.

One semaphore will signal that rendering has finished and can be passed to the
swap chain to be presented.
*/
void RenderCode::createSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS || vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create semaphores");
	}
}

/*
It is possible for the window surface to change such that the swap chain is no longer compatible with it.
One of the reasons that could cause this to happen is the size of the window changing.
This means that the swap chain would have to be recreated each time
*/
void RenderCode::recreateSwapChain()
{

	/*Get the current width and height of the window*/
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	/*If the window is minimized we don't need to recreate the swap chain*/
	if (width == 0 || height == 0)
	{
		return;
	}

	vkDeviceWaitIdle(device); // Wait until all previous resources have been successfuly processed

	cleanupSwapChain(); // Make sure we've fully dealth with the previous swap chain and it's resources

	createSwapChain(); // Create a new swap chain to replace the older one
	createImageViews(); // Image views are based on the swap chain, so they must also be recreated
	createRenderPass(); // The render pass depends on the format of the swap chain
	createGraphicsPipeline(); // We now have to recrtee the entire pipeline as scissor and viewport information is specified here
	createFramebuffers();  // The framebuffer is directily dependent on the swap chain
	createCommandBuffers(); // Same for the command buffer

}

/*
Deals with swap chain resources. Makes recreation of the swap chain easier.
*/
void RenderCode::cleanupSwapChain()
{
	/*We should delete the framebuffers for each image view, and have to do this before the render pass and image views that they are based on*/
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
	}

	/*We only need to clean up the command buffers, and then reuse the command pool for efficiency purposes*/
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

	vkDestroyPipeline(device, graphicsPipeline, nullptr); // Destroy the graphics pipeline information
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr); // Destroy the pipeline layout which contains the layout of constants we'll be passing to the shaders
	vkDestroyRenderPass(device, renderPass, nullptr); //Destroy the render pass, along with it's subpasses and attachment information

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr); // Free the resources for our swap chain
}

/*
Waits for glfw to signal that a resize event
has been triggered
*/
void RenderCode::onWindowResized(GLFWwindow * window, int width, int height)
{
	RenderCode* app = reinterpret_cast<RenderCode*>(glfwGetWindowUserPointer(window));
	app->recreateSwapChain();
}

/*
	Sets up the Vertex buffer information
*/
void RenderCode::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size(); // Size required for allocating the vertex buffer to GPU memory

	VkBuffer stagingBuffer; // temporary buffer in host memory
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data); // Map the vertex data to the buffer
		memcpy(data, vertices.data(), (size_t) bufferSize); // Copy the data to the locaiton we now reference via the data pointer
	vkUnmapMemory(device, stagingBufferMemory); // and then unmap the data

	/*The copy may not happen immediately. One way to handle it is to specify heap memory that is host-coherent as we have above!!!*/
	/*This may lead to worse memory than explicit flushing, but leave that be for now*/

	/*The type of memory that would beused is device memory for the vertex buffer now. This means we cannot map memory using the vertex buffer, but we can get data from the staging buffer?*/
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory); // The vertex buffer will be used as destination buffer for the transfer

	/*Copy from host to device*/
	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	/*Clean up*/
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	/*At this point data will be read from the GPU, unlike vefore when we were storing and reading from CPU-side*/
}

/*
	Finds the correct type of memory required by a call to a memory allocaiton.
*/
uint32_t RenderCode::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{

	/*First we myst query the device to see what types of memory are offered by it's proeprties*/
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties); 

	/*Check for suitable memory type*/
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) // Memory type arrays specify heaps and other properties of that type, which we need to know.
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) // Weird black magic
		{
			return i; // Return index of the memProperties struct which contains everything we need
		}
	}

	// Else throw an exception
	throw std::runtime_error("Failed to find suitable memory type!");
}

void RenderCode::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer & buffer, VkDeviceMemory & bufferMemory)
{

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO; // Vertex buffer structure
	bufferInfo.size = size; // Total memory in bytes of the vertices array. IT would be ...sizeof(Vertex) * sizeof(vertices). So the size of the Vertex struct in bytes multiplied by the amount of individual vertex elements in the array.
	bufferInfo.usage = usage; // The purpose of the buffer, take note that you can have more than a single usage via  bitwise or. In our case we just wanted it to be used for our vertex data.
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Allows the access for the stcuture to only be given to a single queue at a time. In our case this would be just the Graphcis queue.

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vertex buffer!");
	}

	/*Will compute the amount of memory required by our vertex buffer structure*/
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	/*We know the requirements in terms of memory for our application and the supported memory types on the GPU*/
	/*Therefore we can now start allocating the memory on the GPU of our vertex buffer*/
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	allocInfo.allocationSize = memRequirements.size; // The size of the data in the container
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties); // The type of memory allocation

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allcoate vertex buffer memory!");
	}

	/*If memory allocation was succesful we can now associate this memory on the GPU with a handle*/
	vkBindBufferMemory(device, buffer, bufferMemory, 0); // The 4th paramater is some sort of alignment 
}


/*Copies over data from a buffer specifed as source to one specifed as destination. It must pass the amount of data to be transferred*/
void RenderCode::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	/*Memory transfer operations are executed using command Buffers*/
	VkCommandBufferAllocateInfo allocInfo = {};

	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Tell the driver our intent to use this command buffer only a single time

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion); // Copy the region of memory we want

	vkEndCommandBuffer(commandBuffer); // Since we only use a single copy command, once copying has completed, we can terminate the buffer. We won't use it again.

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

}

/*Set up the index buffer*/
void RenderCode::createIndexBuffer()
{

	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size(); // The size in bytes of the index data

	/*We create a temporary buffer once more*/
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory); // Crates the staging buffer

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory); // Notice the usage is set to index buffer usage

	copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

/*
	Tells Vulkan exactly how the data inside the uniform buffer is
	layed out.
*/
void RenderCode::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {}; // Describes each binding. If you bind multiple, every single one of the bindings must be described
	uboLayoutBinding.binding = 0; // The binding used inside the shader
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Tell it is a uniform buffer object
	uboLayoutBinding.descriptorCount = 1;

	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Specifies which shader the descriptor will be referenced in
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1; // If you represent an array of uniform buffers, for example. In this case it's just going to be an array of one element
	layoutInfo.pBindings = &uboLayoutBinding;

	/*Accepts layout information, that contains the array of bindings*/
	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout!");
	}

}

void RenderCode::createUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject); // The buffer size would be the size of the struct

	createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer, uniformBufferMemory);
}

void RenderCode::updateUniformBuffer()
{

	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo = {};

	/*Model matrix*/
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // first param is the rotation matrix we'll apply this to, first operations so it will be identity.
																										// The second paramater is the angle of rotation, the third paramater is the axis around we are applying the rotation

	/*View matrix*/
	ubo.view = glm::lookAt(glm::vec3(20.0f, 150.0f, 60.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // The first paramater is a position vector, for the eye
																													// The second argument is the direction vector, the way we are looking towards ( point)
																													// The third vector defines our "up" direction

	/*Projection matrix*/
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 1000.0f); // Look with a 45-degree field of view., the aspect ratio is the width / height, the near plane is at 0.1f ( should never be 0.0 or less), and far plane is 10.0f

	/*For the aspect ratio, we should use the swap chain extent, which would record new widths and heights upon resizing events from the application have been recognized*/

	ubo.proj[1][1] *= -1; // Because the Y coordinate of the clip coordinates is flipped? So we flip the scaling factor for the Y axis 

	/*Copy the data in the uniform buffer object*/

	void* data;
	vkMapMemory(device, uniformBufferMemory, 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, uniformBufferMemory);
}

void RenderCode::createDescriptorPool()
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Does this type apply for the whole pool? So will we need separate pools for each type???
	poolSize.descriptorCount = 1; // A single uniform buffer is being used 

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;

	/*Maxmimum number of descriptor sets*/
	poolInfo.maxSets = 1;

	/*Create descriptor pool*/
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool!");
	}

}

void RenderCode::createDescriptorSet()
{
	VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};

	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) // Descriptor sets will be automatically freed when the pool is freed
	{
		throw std::runtime_error("Failed to allocate descriptor set!");
	}

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = uniformBuffer; // The buffer name
	bufferInfo.offset = 0; 
	bufferInfo.range = sizeof(UniformBufferObject); // The size of the structure which hosts the data

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet; // The descriptor set to update
	descriptorWrite.dstBinding = 0; // The binding of the descriptor set we are udpating
	descriptorWrite.dstArrayElement = 0;

	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // uniform buffer is the type of the descriptor we're going to change
	descriptorWrite.descriptorCount = 1; // The amount of array elements we want to change. We only have one, so we set it to one.

	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr;
	descriptorWrite.pTexelBufferView;

	vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr); // Update the descriptors


}



/*
The first paramater sets a flag for the type of the debug message. In other words if it's an error, warning or report.

The object type represets a Vulkan type. This can be anything of the Vulkan library structs VkPhysicalDEvice etc. All Vulkan handles can be represented by a uint64_t.

The msg paramater is a pointer to the message itself.

The last paramater us data we can pass to the debug callback.

The callback returns a boolean. This boolean, if true, tells the API whether or not to abort the function that triggered the flag.
This is because not all errors will be fatal ( result in a crash). If the returned value is true then we also abort the call that triggered it with the VK_ERROR_VALIDATION_FAILED_EXT.
error.

This is used to test only validation layers themselves, hence the function should always return FALSE by default.
*/
VKAPI_ATTR VkBool32 VKAPI_CALL RenderCode::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t locaiton, int32_t code, const char * layerPrefix, const char * msg, void * userData)
{
	std::cerr << "Validation layer: " << msg << std::endl;
	return VK_FALSE;
}

/*
This function runs our entire application.
*/
void RenderCode::run()
{
	initWindow(); // Initializes Window api
	initVulkan(); // Initializes Vulkan isntances
	mainLoop(); // Runs the main render loop
	cleanup(); // Frees resource that were allocated
}

/*The constructor and destructor do nothing*/
RenderCode::RenderCode() : physicalDevice(VK_NULL_HANDLE) // Initally no device is bound to our application
{
}

RenderCode::RenderCode(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) : vertices(vertices), indices(indices)
{
}


RenderCode::~RenderCode()
{
}


