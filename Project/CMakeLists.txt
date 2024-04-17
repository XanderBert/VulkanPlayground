#Copy obj files to the binary directory
set(MODEL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Assets")
file(GLOB_RECURSE MODELS "${MODEL_SOURCE_DIR}/*")
file(COPY ${MODELS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

#Copy the imgui.ini file to the binary directory
set(IMGUI_INI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui.ini")
file(GLOB_RECURSE IMGUI_INI "${IMGUI_INI_SOURCE_DIR}")
file(COPY ${IMGUI_INI} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Shaders
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
set(BASE_SHADERS_DIR ${SHADER_SOURCE_DIR})
set(BASE_SHADER_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(GLOB BASE_SHADERS
"${BASE_SHADERS_DIR}/*frag"
"${BASE_SHADERS_DIR}/*vert")

file(COPY ${BASE_SHADERS} DESTINATION ${BASE_SHADER_BIN_DIR})

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert"
)



#Compile Shaders
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

#Add cpp and header files
set(SOURCES
    "main.cpp"
    "vulkanbase/VulkanBase.h" 
    "vulkanbase/VulkanUtil.h"
    "labwork/Week05.cpp"
    "labwork/Week06.cpp"
    "shaders/Logic/Shader.h"
    "shaders/Logic/Shader.cpp" 
    "Patterns/Singleton.h"
    "LuaScripts/LuaScriptRunner.h"
    "shaders/Logic/ShaderFactory.h"
    "shaders/Logic/SpirvHelper.h"
    "shaders/Logic/ShaderFileWatcher.h" 
    "Mesh/Vertex.h" 
    "Mesh/Mesh.h" 
    "Mesh/Mesh.cpp" 
    "Core/CommandBuffer.h" 
    "Core/CommandBuffer.cpp" 
    "Core/CommandPool.h" 
    "Core/CommandPool.cpp" 
    "Core/QueueFamilyIndices.h" 
    "Core/GraphicsPipeline.h" 
    "Core/GraphicsPipeline.cpp" 
    "Patterns/ServiceLocator.h" 
    "vulkanbase/VulkanTypes.h" 
    "Scene/Scene.h" 
    "Scene/Scene.cpp" 
    "Core/ImGuiWrapper.h" 
    "Mesh/Material.h" 
    "Core/Buffer.h" 
    "Core/Buffer.cpp" 
    "Core/Logger.h" 
    "Camera/Camera.h" 
    "Camera/Camera.cpp"
    "Core/Descriptor.h"
    "Core/Descriptor.cpp" 
    "Mesh/Material.cpp" 
    "Input/Input.h" 
    "Mesh/ModelLoader.h" 
    "Core/DepthResource.h" 
    "Mesh/MaterialManager.h" 
    "Mesh/ModelLoader.cpp"
    "shaders/Logic/SpirvHelper.h"
    "vulkanbase/VulkanUtil.cpp" 
    "Core/SwapChain.h" 
    "Core/ImGuiWrapper.cpp" )


include(FetchContent)

FetchContent_Declare(
  ImGui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.90.5-docking
)
FetchContent_MakeAvailable(ImGui)
set(IMGUI_DIR ${imgui_SOURCE_DIR})
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)
set (IMGUI_SOURCE_FILES
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp 
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp 
    ${IMGUI_DIR}/imgui.cpp 
    ${IMGUI_DIR}/imgui_draw.cpp 
    ${IMGUI_DIR}/imgui_demo.cpp 
    ${IMGUI_DIR}/imgui_tables.cpp 
    ${IMGUI_DIR}/imgui_widgets.cpp
)

FetchContent_Declare(
  Sol
  GIT_REPOSITORY https://github.com/ThePhD/sol2.git
  GIT_TAG        v4.0.0-alpha
)
FetchContent_MakeAvailable(Sol)
set(SOL_DIR ${sol_SOURCE_DIR})
include_directories(${SOL_DIR}/single/include)

FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/marovira/lua
    GIT_TAG         5.4.4
)
FetchContent_MakeAvailable(lua)
set (LUA_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/LuaScripts/CreateShader.lua)
file(COPY ${LUA_SCRIPTS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/LuaScripts)

FetchContent_Declare(
    tinyObj
    GIT_REPOSITORY  https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG         v2.0.0rc13
)
FetchContent_MakeAvailable(tinyObj)
set(TINYOBJ_DIR ${tinyObj_SOURCE_DIR})

FetchContent_Declare(
    glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG         1.0.1
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    ImGuizmo
    GIT_REPOSITORY  https://github.com/CedricGuillemet/ImGuizmo.git
)
FetchContent_MakeAvailable(ImGuizmo)
set (IMGUIZMO_SOURCE_FILES
    ${imguizmo_SOURCE_DIR}/GraphEditor.cpp 
    ${imguizmo_SOURCE_DIR}/GraphEditor.h
    ${imguizmo_SOURCE_DIR}/ImCurveEdit.cpp
    ${imguizmo_SOURCE_DIR}/ImCurveEdit.h
    ${imguizmo_SOURCE_DIR}/ImGradient.cpp
    ${imguizmo_SOURCE_DIR}/ImGradient.h
    ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp 
    ${imguizmo_SOURCE_DIR}/ImGuizmo.h
    ${imguizmo_SOURCE_DIR}/ImSequencer.cpp
    ${imguizmo_SOURCE_DIR}/ImSequencer.h
    ${imguizmo_SOURCE_DIR}/ImZoomSlider.h
)

FetchContent_Declare(
    ImPlot
    GIT_REPOSITORY  https://github.com/epezent/implot.git
)
FetchContent_MakeAvailable(ImPlot)
set (IMPLOT_SOURCE_FILES
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot.h
    ${implot_SOURCE_DIR}/implot_items.cpp
    ${implot_SOURCE_DIR}/implot_internal.h
)



set(EXTERNALS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../includes)


# Create the executable
add_executable(
    ${PROJECT_NAME} 
    ${SOURCES} 
    ${GLSL_SOURCE_FILES}  
    ${IMGUI_SOURCE_FILES} 
    ${IMGUIZMO_SOURCE_FILES} 
    ${IMPLOT_SOURCE_FILES} 
    ${LUA_SCRIPTS} 
)



add_dependencies(${PROJECT_NAME} 
Shaders
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} 
    PRIVATE ${EXTERNALS_DIR}
    PRIVATE ${LUA_DIR}
    PRIVATE ${TINY_OBJ_DIR}
    PRIVATE ${IMGUI_DIR}
    PRIVATE ${imguizmo_SOURCE_DIR}
    PRIVATE ${implot_SOURCE_DIR}
)

string(REPLACE ".lib" "d.lib" SHADERC_DEBUG ${Vulkan_shaderc_combined_LIBRARY})

target_link_options(${PROJECT_NAME}  PUBLIC "/ignore:4099")

target_link_libraries(${PROJECT_NAME} 
    
PRIVATE ${Vulkan_LIBRARIES}
    PUBLIC glfw
    PUBLIC debug ${SHADERC_DEBUG} 
    PRIVATE lua::lua
    PRIVATE efsw-static
    PRIVATE glm
    PRIVATE tinyobjloader
)