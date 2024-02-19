cmake_minimum_required(VERSION 3.10)

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


set (IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Project/vulkanbase/imgui)

# Fetch ImGui
FetchContent_Populate(imgui
  URL https://github.com/ocornut/imgui/archive/docking.zip
  SOURCE_DIR ${IMGUI_DIR}
)

include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)


#target_link_libraries(imgui_vulkan PUBLIC glfw Vulkan::Vulkan)
#set(LIBRARIES "glfw;Vulkan::Vulkan")
#add_executable(imgui_vulkan ${sources} ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp)
#target_link_libraries(imgui_vulkan ${LIBRARIES})
#target_compile_definitions(imgui_vulkan PUBLIC -DImTextureID=ImU64)


# If using validation layers, copy the required JSON files (optional)
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${CMAKE_SOURCE_DIR}/path_to_json/VK_LAYER_KHRONOS_validation.json"
#        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
