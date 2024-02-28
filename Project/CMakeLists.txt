# Source files
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

# Copy the base shaders to the binary directory
set(BASE_SHADERS_DIR ${SHADER_SOURCE_DIR}/BaseShaders)
set(BASE_SHADER_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders/BaseShaders)
file(GLOB_RECURSE BASE_SHADERS "${BASE_SHADERS_DIR}/*")
file(COPY ${BASE_SHADERS} DESTINATION ${BASE_SHADER_BIN_DIR})

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)


add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)

set(SOURCES
    "main.cpp"
    "vulkanbase/VulkanBase.h" 
    "vulkanbase/VulkanUtil.h"
    "labwork/Week02.cpp" 
    "labwork/Week03.cpp" 
    "labwork/Week04.cpp" 
    "labwork/Week05.cpp"
    "labwork/Week06.cpp"
    "shaders/Shader.h"
    "shaders/Shader.cpp" 
    "Patterns/Singleton.h"
    "LuaScripts/LuaScriptRunner.h"
    "shaders/ShaderFactory.h"
    "shaders/SpirvHelper.h"
)


set(EXTERNALS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../includes)

#Set ImGui Source Files
set(IMGUI_DIR ${EXTERNALS_DIR}/imgui)
set (IMGUI_SOURCE_FILES
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp 
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp 
    ${IMGUI_DIR}/imgui.cpp 
    ${IMGUI_DIR}/imgui_draw.cpp 
    ${IMGUI_DIR}/imgui_demo.cpp 
    ${IMGUI_DIR}/imgui_tables.cpp 
    ${IMGUI_DIR}/imgui_widgets.cpp
)

# Copy the Lua script to the source directory
set (LUA_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/LuaScripts/CreateShader.lua)
file(COPY ${LUA_SCRIPTS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/LuaScripts)


# Create the executable
add_executable(
    ${PROJECT_NAME} 
    ${SOURCES} 
    ${GLSL_SOURCE_FILES}  
    "vulkanbase/VulkanUtil.cpp" 
    "labwork/Week03.cpp" 
    ${IMGUI_SOURCE_FILES} 
    ${LUA_SCRIPTS} 
    "shaders/SpirvHelper.h")



add_dependencies(${PROJECT_NAME} Shaders)
# Link libraries
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} 
    PRIVATE ${EXTERNALS_DIR}
    PRIVATE ${LUA_DIR}
    PRIVATE ${IMGUI_DIR}
)

string(REPLACE ".lib" "d.lib" SHADERC_DEBUG ${Vulkan_shaderc_combined_LIBRARY})
target_link_libraries(
    ${PROJECT_NAME} 
    PRIVATE ${Vulkan_LIBRARIES}
    PUBLIC debug ${SHADERC_DEBUG} optimized ${Vulkan_shaderc_combined_LIBRARY}
    glfw 
    lua
)