cmake_minimum_required(VERSION 3.27)

# Project Name
project(VulkanPlayground)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

#Fetch GLFW
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8 # Specify the version you need
)
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)
FetchContent_MakeAvailable(glfw)


# Find the required packages
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)


#set flags
#set(GCC_COVERAGE_COMPILE_FLAGS "-fpermissive")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

#set sanitisers only in debug mode

#if (MSVC)
#    add_compile_options(/W4 /WX)
#else()
#    add_compile_options(-Wall -Wextra -pedantic -Werror)
#endif()

# Include Directories
include_directories(${Vulkan_INCLUDE_DIRS})
add_subdirectory(Project)
add_subdirectory(includes)


#find_package(VulkanMemoryAllocator CONFIG REQUIRED)
# If using validation layers, copy the required JSON files (optional)
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${CMAKE_SOURCE_DIR}/path_to_json/VK_LAYER_KHRONOS_validation.json"
#        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

