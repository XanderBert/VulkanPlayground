#define NO_DEBUG_HEAP = 1
#define NOMINMAX
//#define GLM_ENABLE_EXPERIMENTAL 1
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "Core/Logger.h"
#include "vulkanbase/VulkanBase.h"

int main()
{
	// DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1 = 1
	// ISABLE_LAYER_NV_OPTIMUS_1 = 1
	// _putenv_s("DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1", "1");
	// _putenv_s("DISABLE_LAYER_NV_OPTIMUS_1", "1");
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