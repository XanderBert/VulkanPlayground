#pragma once
#include "ColorAttachment.h"
#include "DepthResource.h"

class VulkanContext;


class GBuffer
{
public:
	GBuffer() = default;
	~GBuffer() = default;
	GBuffer(const GBuffer&) = delete;
	GBuffer& operator=(const GBuffer&) = delete;
	GBuffer(GBuffer&&) = delete;
	GBuffer& operator=(GBuffer&&) = delete;

	static void Init(const VulkanContext* vulkanContext);
	static void Cleanup(const VulkanContext* vulkanContext);

	static void BindDepth(Descriptor::DescriptorWriter& writer, int depthBinding);
	static void BindNormal(Descriptor::DescriptorWriter& writer, int normalBinding);
	static void BindAlbedo(Descriptor::DescriptorWriter& writer, int albedoBinding);

	static ColorAttachment* GetColorAttachmentNormal();
	static DepthAttachment* GetDepthAttachment();

	static ColorAttachment* GetAlbedoAttachment();

	static void OnImGui();

private:
	inline static ColorAttachment m_ColorAttachmentNormal;
	inline static DepthAttachment m_DepthAttachment;

	inline static ColorAttachment m_AlbedoAttachment;
};
