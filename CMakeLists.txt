cmake_minimum_required(VERSION 3.27)

# Project Name
project(VulkanPlayground)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(FETCHCONTENT_QUIET FALSE)  # Force verbose output to debug fetch content issues
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


# Include Directories
include_directories(${Vulkan_INCLUDE_DIRS})
add_subdirectory(Project)
add_subdirectory(includes)