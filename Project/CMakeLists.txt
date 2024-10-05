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
"${SHADER_SOURCE_DIR}/*.glsl")

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

string(REPLACE ".lib" "d.lib" SHADERC_DEBUG ${Vulkan_shaderc_combined_LIBRARY})
target_link_libraries(${PROJECT_NAME}
    PRIVATE ${Vulkan_LIBRARIES}
    PUBLIC glfw
    PUBLIC debug ${SHADERC_DEBUG} optimized ${Vulkan_shaderc_combined_LIBRARY}
    PRIVATE efsw
    PRIVATE glm
    PRIVATE tinyobjloader
    PRIVATE fastgltf::fastgltf
)