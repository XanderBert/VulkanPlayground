#define _NO_DEBUG_HEAP = 1
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE




#include "vulkanbase/VulkanBase.h"
int main()
{
	//DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1 = 1
	//DISABLE_LAYER_NV_OPTIMUS_1 = 1
	//_putenv_s("DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1", "1");
	//_putenv_s("DISABLE_LAYER_NV_OPTIMUS_1", "1");
	VulkanBase app;

	try 
	{
		app.run();
	}
	catch (const std::exception& e) 
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}