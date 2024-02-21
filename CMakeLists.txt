cmake_minimum_required(VERSION 3.27)

# Project Name
project(VulkanPlayground)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

#Fetch GLFW
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8 # Specify the version you need
)
FetchContent_MakeAvailable(glfw)



# Find the required packages
find_package(Vulkan REQUIRED)

# Include Directories
include_directories(${Vulkan_INCLUDE_DIRS})
add_subdirectory(Project)


set (IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes/imgui)
# Fetch ImGui
#FetchContent_Populate(imgui
#  URL https://github.com/ocornut/imgui/archive/docking.zip
#  SOURCE_DIR ${IMGUI_DIR}
#)

include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)


# If using validation layers, copy the required JSON files (optional)
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${CMAKE_SOURCE_DIR}/path_to_json/VK_LAYER_KHRONOS_validation.json"
#        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
