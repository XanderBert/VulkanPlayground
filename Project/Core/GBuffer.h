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

	static ColorAttachment* GetColorAttachmentNormal();
	static DepthAttachment* GetDepthAttachment();

	static void OnImGui();

private:
	inline static ColorAttachment m_ColorAttachmentNormal;
	inline static DepthAttachment m_DepthAttachment;
};
