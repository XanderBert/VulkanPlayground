# VulkanPlayground

![Skybox](gltf.gif)


## Some Features
- Lua Script Support
- Runtime Shader Creation
- Runtime Shader Compiling
- Runtime Shader Changes watcher thread -> automatically compiles the changed shader in runtime
- PBR Material System (Channel and non channel packed)
- Runtime GLTF/Obj Model Loading
- Runtime Texture Loading/Swapping
- .ktx Texture Loading

## What do i want to get out of this project?
First of all i want to setup a working runtime shader editor and abstract vulkan.

I would like to have support for multiple Shader languages. The first one on the list is Slang (next to GLSL).

![HotShaderReload](HotShaderReload.gif)

TODO:
- Rework Render System to have better material instancing
- Shadow Mapping
- Deferred Rendering
- Object Picking for Object Manipulation
- Scene Graph
- Scene Serialization
- Scene Deserialization
- Make 2 types of Descriptor Allocators (Each Frame (Dynamic Descriptor Sets) and One Time (Static Descriptor Sets))