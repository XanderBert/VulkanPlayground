#define NO_DEBUG_HEAP = 1
#define NOMINMAX
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE



#include "Core/Logger.h"
#include "vulkanbase/VulkanBase.h"

int main()
{
	VulkanBase app;
	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
	    LogError(e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}