# Source files
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

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
    # Add other source files here
    "labwork/Week01.cpp"
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

#Add the Lua Library
set (LUA_DIR ${EXTERNALS_DIR}/lua)
add_library(
    lua STATIC
    ${LUA_DIR}/lapi.c
    ${LUA_DIR}/lauxlib.c
    ${LUA_DIR}/lbaselib.c
    ${LUA_DIR}/lcode.c
    ${LUA_DIR}/lcorolib.c
    ${LUA_DIR}/lctype.c
    ${LUA_DIR}/ldblib.c
    ${LUA_DIR}/ldebug.c
    ${LUA_DIR}/ldo.c
    ${LUA_DIR}/ldump.c
    ${LUA_DIR}/lfunc.c
    ${LUA_DIR}/lgc.c
    ${LUA_DIR}/linit.c
    ${LUA_DIR}/liolib.c
    ${LUA_DIR}/llex.c
    ${LUA_DIR}/lmathlib.c
    ${LUA_DIR}/lmem.c
    ${LUA_DIR}/loadlib.c
    ${LUA_DIR}/lobject.c
    ${LUA_DIR}/lopcodes.c
    ${LUA_DIR}/loslib.c
    ${LUA_DIR}/lparser.c
    ${LUA_DIR}/lstate.c
    ${LUA_DIR}/lstring.c
    ${LUA_DIR}/lstrlib.c
    ${LUA_DIR}/ltable.c
    ${LUA_DIR}/ltablib.c
    ${LUA_DIR}/ltm.c
    ${LUA_DIR}/lua.c
    ${LUA_DIR}/lundump.c
    ${LUA_DIR}/lutf8lib.c
    ${LUA_DIR}/lvm.c
    ${LUA_DIR}/lzio.c
)

# Copy the Lua script to the source directory
set (LUA_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/LuaScripts/CreateShader.lua)
file(COPY ${LUA_SCRIPTS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/LuaScripts)

# Copy the base shaders to the binary directory
set (BASE_SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag)
file(COPY ${BASE_SHADERS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)


# Create the executable
add_executable(
    ${PROJECT_NAME} 
    ${SOURCES} 
    ${GLSL_SOURCE_FILES}  
    "vulkanbase/VulkanUtil.cpp" 
    "labwork/Week03.cpp" 
    ${IMGUI_SOURCE_FILES} 

    ${LUA_SCRIPTS})



add_dependencies(${PROJECT_NAME} Shaders)
# Link libraries
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} 
    PRIVATE ${EXTERNALS_DIR}
    PRIVATE ${LUA_DIR}
    PRIVATE ${IMGUI_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} glfw lua)