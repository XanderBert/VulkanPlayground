#Copy obj files to the binary directory
set(MODEL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Assets")
file(GLOB_RECURSE MODELS "${MODEL_SOURCE_DIR}/*")
file(COPY ${MODELS} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Assets")

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
"${BASE_SHADERS_DIR}/*vert"
"${SHADER_SOURCE_DIR}/*.glsl"
"${SHADER_SOURCE_DIR}/*.comp")

file(COPY ${BASE_SHADERS} DESTINATION ${BASE_SHADER_BIN_DIR})

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert"
    "${SHADER_SOURCE_DIR}/*.comp"
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
    "Core/ImGuiWrapper.cpp"  
    "Core/Image/ImageLoader.cpp" 
    "Core/Image/ImageLoader.h" 
    "Core/GlobalDescriptor.h"
    "Core/GlobalDescriptor.cpp" 
    "Core/DepthResource.cpp" 
    "Core/DynamicUniformBuffer.h"
    "Core/DynamicUniformBuffer.cpp"
    "Core/DescriptorSet.cpp"
    "Core/DescriptorSet.h"
        Scene/SceneManager.cpp
        Scene/SceneManager.h
        Core/Image/Texture.cpp
        Core/Image/Texture.h
        Core/Lights/Light.cpp
        Core/Lights/Light.h
        Core/Lights/LightManager.cpp
        Core/Lights/LightManager.h
        Patterns/Delegate.h
        shaders/Logic/ShaderEditor.cpp
        shaders/Logic/ShaderEditor.h
        vulkanbase/VulkanBase.cpp
        vulkanbase/VulkanTypes.cpp
        Core/GBuffer.cpp
        Core/GBuffer.h
        Core/ColorAttachment.cpp
        Core/ColorAttachment.h
        Types/CircularBuffer.h
        Timer/TimerGraph.cpp
        Timer/TimerGraph.h
)


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
    tinyObj
    GIT_REPOSITORY  https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG         v2.0.0rc13
)
FetchContent_MakeAvailable(tinyObj)
set(TINYOBJ_DIR ${tinyObj_SOURCE_DIR})


#spirv-headers (NEEDED ONLY FOR SPIRV-TOOLS)
FetchContent_Declare(
        spirv-headers
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
        GIT_TAG vulkan-sdk-1.3.290.0
)
FetchContent_MakeAvailable(spirv-headers)

#spirv-tools (NEEDED ONLY FOR SHADERC)
FetchContent_Declare(
        spirv-tools
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
        GIT_TAG vulkan-sdk-1.3.290
)
FetchContent_MakeAvailable(spirv-tools)

FetchContent_Declare(
        glslang
        GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
        GIT_TAG vulkan-sdk-1.3.290.0
)
FetchContent_MakeAvailable(glslang)

#shaderc
FetchContent_Declare(
        shaderc
        GIT_REPOSITORY https://github.com/google/shaderc.git
        GIT_TAG v2024.0
)
set(SHADERC_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(shaderc)






FetchContent_Declare(
    glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG         1.0.1
)
option(GLM_TEST_ENABLE OFF)
option(GLM_FORCE_RADIANS ON)
option(GLM_FORCE_DEPTH_ZERO_TO_ONE ON)
option(GLM_ENABLE_CXX_20 ON)
option(GLM_ENABLE_SIMD_SSE4_2 ON)
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

FetchContent_Declare(
        ImGuiFileDialog
        GIT_REPOSITORY  https://github.com/aiekick/ImGuiFileDialog.git
)
FetchContent_MakeAvailable(ImGuiFileDialog)
set (IMGUIFILEDIALOG_SOURCE_FILES
        ${imguifiledialog_SOURCE_DIR}/ImGuiFileDialog.cpp
        ${imguifiledialog_SOURCE_DIR}/ImGuiFileDialog.h
        ${imguifiledialog_SOURCE_DIR}/ImGuiFileDialogConfig.h
)



set(EXTERNALS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../includes)
set(STB_SOURCES ${EXTERNALS_DIR}/stb/stb_image.h)
include_directories(${EXTERNALS_DIR}/stb)

set(KTX_DIR ${EXTERNALS_DIR}/ktx)
set(KTX_SOURCES
        ${KTX_DIR}/lib/texture.c
        ${KTX_DIR}/lib/hashlist.c
        ${KTX_DIR}/lib/checkheader.c
        ${KTX_DIR}/lib/swap.c
        ${KTX_DIR}/lib/memstream.c
        ${KTX_DIR}/lib/filestream.c
        ${KTX_DIR}/lib/vk_format.h
        ${KTX_DIR}/lib/vk_funcs.c
        ${KTX_DIR}/lib/vk_funcs.h
        ${KTX_DIR}/lib/vkloader.c
)
include_directories( ${KTX_DIR}/include)
include_directories( ${KTX_DIR}/other_include)
#include_directories( ${KTX_DIR}/utils)


set(ImGuiTextEditSourceDir ${EXTERNALS_DIR}/ImGuiTextEdit)

set(IMGUI_TEXTEDIT_SOURCES
    ${ImGuiTextEditSourceDir}/TextEditor.cpp
    ${ImGuiTextEditSourceDir}/TextEditor.h)

include_directories(${ImGuiTextEditSourceDir})

set(VMA_SOURCE_DIR ${EXTERNALS_DIR}/VulkanMemoryAllocator)
set(VMA_SOURCES
    ${VMA_SOURCE_DIR}/vk_mem_alloc.h
        Core/VmaUsage.h
        Core/VmaUsage.cpp
)

# Create the executable
message("IMGUI_DIR: ${IMGUI_DIR}")
message("IMGUI_SOURCE_FILES: ${IMGUI_SOURCE_FILES}")

add_executable(
    ${PROJECT_NAME} 
    ${SOURCES} 
    ${GLSL_SOURCE_FILES}  
    ${IMGUI_SOURCE_FILES} 
    ${IMGUIZMO_SOURCE_FILES} 
    ${IMPLOT_SOURCE_FILES}
    ${IMGUIFILEDIALOG_SOURCE_FILES}
    ${STB_SOURCES}
    ${KTX_SOURCES}
    ${IMGUI_TEXTEDIT_SOURCES}
    ${VMA_SOURCES}
)




add_dependencies(${PROJECT_NAME} 
Shaders
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} 
    PRIVATE ${EXTERNALS_DIR}
    PRIVATE ${TINY_OBJ_DIR}
    PRIVATE ${IMGUI_DIR}
    PRIVATE ${imguizmo_SOURCE_DIR}
    PRIVATE ${implot_SOURCE_DIR}
    PRIVATE ${imguifiledialog_SOURCE_DIR}
    PRIVATE ${VMA_SOURCE_DIR}
)




# For Windows, rename to debug libraries with .d.lib extension
if(WIN32)
    string(REPLACE ".lib" "d.lib" SHADERC_DEBUG ${Vulkan_shaderc_combined_LIBRARY})
endif()


target_link_libraries(${PROJECT_NAME}
    PRIVATE ${Vulkan_LIBRARIES}
    PRIVATE glfw
    PRIVATE shaderc_combined
    PRIVATE efsw
    PRIVATE glm
    PRIVATE tinyobjloader
    PRIVATE fastgltf::fastgltf
)