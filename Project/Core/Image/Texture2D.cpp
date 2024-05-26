

#include "Core/CommandBuffer.h"
#include "ImageLoader.h"
#include "Texture2D.h"

#include <filesystem>

#include "Patterns/ServiceLocator.h"
#include "cmake-build-release-visual-studio/_deps/imguifiledialog-src/ImGuiFileDialog.h"


Texture2D::Texture2D(const std::filesystem::path &path, ::VulkanContext *vulkanContext, ColorType colorType) :
    Texture(path, vulkanContext, colorType)

{
    InitTexture(std::nullopt);
}
Texture2D::Texture2D(const LoadedImage &loadedImage, ::VulkanContext *vulkanContext, ColorType colorType) :
    Texture(loadedImage, vulkanContext, colorType)
{
    InitTexture(loadedImage);
}

Texture2D::Texture2D(Texture2D &&other) noexcept
    : Texture(std::move(other))
{
    m_ImTexture = std::move(other.m_ImTexture);
}

void Texture2D::TransitionAndCopyImageBuffer(VkBuffer srcBuffer) {
    // Create Command Buffer
    CommandBuffer commandBuffer{};
    CommandBufferManager::CreateCommandBufferSingleUse(VulkanContext, commandBuffer);

    std::vector<VkBufferImageCopy> bufferCopyRegions;

    // TODO Check Mips && offset
    uint32_t offset = 0;
    for (uint32_t mipLevel{}; mipLevel < m_MipLevels; ++mipLevel) {
        VkBufferImageCopy region{};
        region.bufferOffset = offset;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = mipLevel;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageExtent.width = m_ImageSize.x >> mipLevel;
        region.imageExtent.height = m_ImageSize.y >> mipLevel;
        region.imageExtent.depth = 1;

        bufferCopyRegions.push_back(region);
    }


    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = m_MipLevels;
    subresourceRange.layerCount = static_cast<uint32_t>(bufferCopyRegions.size() / m_MipLevels);

    // Transition the image to transfer destination
    tools::InsertImageMemoryBarrier(commandBuffer.Handle, m_Image, VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

    vkCmdCopyBufferToImage(commandBuffer.Handle, srcBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

    // Transition the image to shader read
    tools::InsertImageMemoryBarrier(commandBuffer.Handle, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);


    CommandBufferManager::EndCommandBufferSingleUse(VulkanContext, commandBuffer);
}
void Texture2D::Cleanup(VkDevice device) const
{
    m_ImTexture->Cleanup();
    Texture::Cleanup(device);
}

void Texture2D::OnImGui()
{
    // Try to reload the texture
    const std::string labelAddition = "##" + std::to_string(reinterpret_cast<uintptr_t>(this));
    const std::string dialogLabel = "Choose Texture" + labelAddition;
    const std::string label = "Change Texture" + labelAddition;
    if (ImGui::Button(label.c_str()))
    {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_Modal;


        ImGuiFileDialog::Instance()->OpenDialog(dialogLabel.c_str(), "Choose Texture", ".png", config);
        ImGui::SetNextWindowSize({300,200});
    }



    if (ImGuiFileDialog::Instance()->Display(dialogLabel.c_str()))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetCurrentFileName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            m_Path = filePath + "\\" + filePathName;

            Cleanup(VulkanContext->device);
            InitTexture(std::nullopt);
        }
        ImGuiFileDialog::Instance()->Close();
    }


    ImGui::SameLine();
    m_ImTexture->OnImGui();
}


void Texture2D::InitTexture(std::optional<LoadedImage> loadedImage)
{
    std::pair<VkBuffer, VkDeviceMemory> stagingSources;

    if(!loadedImage.has_value())
    {
        //Check if is .ktx file
        if (m_Path.extension() == ".ktx")
        {
            ktxTexture* texture{};
            stagingSources = ktx::CreateImage(VulkanContext, m_Path, m_ImageSize, m_MipLevels, &texture);
            ktxTexture_Destroy(texture);
        }
        else
        {
            stagingSources = stbi::CreateImage(VulkanContext, m_Path, m_ImageSize, m_MipLevels);
        }
    }
    else
    {
        stagingSources.first = loadedImage->staginBuffer;
        stagingSources.second = loadedImage->stagingBufferMemory;
    }


    Image::CreateImage(VulkanContext, m_ImageSize.x, m_ImageSize.y, m_MipLevels, VK_SAMPLE_COUNT_1_BIT, static_cast<VkFormat>(m_ColorType), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory);

    TransitionAndCopyImageBuffer(stagingSources.first);
    vkDestroyBuffer(VulkanContext->device, stagingSources.first, nullptr);
    vkFreeMemory(VulkanContext->device, stagingSources.second, nullptr);

    Image::CreateImageView(VulkanContext->device, m_Image, static_cast<VkFormat>(m_ColorType), VK_IMAGE_ASPECT_COLOR_BIT,m_ImageView);
    Image::CreateSampler(VulkanContext, m_Sampler, m_MipLevels);


    //Setup ImGui Texture Rendering
    ImVec2 imageSize = ImVec2(m_ImageSize.x / 5.0f, m_ImageSize.y / 5.0f);
    m_ImTexture = std::make_unique<ImGuiTexture>(m_Sampler, m_ImageView, imageSize);
}
