#include "Texture.h"

#include <filesystem>
#include <utility>

#include "vulkanbase/VulkanTypes.h"
#include "Core/CommandBuffer.h"
#include "ImGuiFileDialog.h"


Texture::Texture(const std::variant<std::filesystem::path,ImageInMemory>& pathOrImage, VulkanContext *vulkanContext, ColorType colorType, TextureType textureType)
: m_pContext(vulkanContext)
, m_ColorType(colorType)
, m_TextureType(textureType)
{
    std::visit([this](auto&& arg)
    {
        this->InitTexture(arg);
    }, pathOrImage);
}


void Texture::OnImGui()
{
    if(m_TextureType == TextureType::TEXTURE_CUBE) return;

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


        ImGuiFileDialog::Instance()->OpenDialog(dialogLabel, "Choose Texture", ".png", config);
        ImGui::SetNextWindowSize({300,200});
    }



    if (ImGuiFileDialog::Instance()->Display(dialogLabel))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetCurrentFileName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

            Cleanup(m_pContext->device);
            InitTexture(filePath + "\\" + filePathName);
        }
        ImGuiFileDialog::Instance()->Close();
    }


    ImGui::SameLine();
    m_ImGuiTexture->OnImGui();
}

void Texture::ProperBind(int bindingNumber, Descriptor::DescriptorWriter &descriptorWriter) const
{
    descriptorWriter.WriteImage(bindingNumber, m_ImageView, m_Sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}



void Texture::Cleanup(VkDevice device) const
{
    if(m_TextureType != TextureType::TEXTURE_CUBE)
    {
        m_ImGuiTexture->Cleanup();
    }


    vkDestroySampler(device, m_Sampler, nullptr);
    vkDestroyImageView(device, m_ImageView, nullptr);
    vmaDestroyImage(Allocator::VmaAllocator, m_Image, m_ImageMemory);
}

void Texture::InitTexture(const ImageInMemory &loadedImage)
{
    m_ImageSize = loadedImage.imageSize;
    m_MipLevels = loadedImage.mipLevels;

    Image::CreateImage(m_ImageSize.x, m_ImageSize.y, m_MipLevels, VK_SAMPLE_COUNT_1_BIT, static_cast<VkFormat>(m_ColorType), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, m_Image, m_ImageMemory, m_TextureType);

    TransitionAndCopyImageBuffer(loadedImage.staginBuffer, loadedImage.texture);

    if(loadedImage.texture.has_value()) ktxTexture_Destroy(loadedImage.texture.value());

    vmaDestroyBuffer(Allocator::VmaAllocator, loadedImage.staginBuffer, loadedImage.stagingBufferMemory);

    Image::CreateImageView(m_pContext->device, m_Image, static_cast<VkFormat>(m_ColorType), VK_IMAGE_ASPECT_COLOR_BIT,m_ImageView, m_TextureType);

    Image::CreateSampler(m_pContext, m_Sampler, m_MipLevels);


    //Setup the ImGuiTexture
    if(m_TextureType == TextureType::TEXTURE_CUBE) return;
    m_ImGuiTexture = std::make_unique<ImGuiTexture>(m_Sampler, m_ImageView, ImVec2(m_ImageSize.x, m_ImageSize.y));
}

void Texture::InitTexture(const std::filesystem::path &path)
{
    m_Path = VulkanContext::GetAssetPath(path.generic_string());

    ImageInMemory stagingSources;
    std::pair<VkBuffer, VmaAllocation> imageData;

    //Note: For now we only support .ktx files for a cubemap
    if (m_Path.value().extension() == ".ktx" || m_TextureType == TextureType::TEXTURE_CUBE)
    {
        ktxTexture* texture = nullptr;
        imageData = ktx::CreateImage(m_Path.value(), stagingSources.imageSize, stagingSources.mipLevels, &texture);
        stagingSources.texture = texture;
    }
    else
    {
        imageData = stbi::CreateImage(m_Path.value(), stagingSources.imageSize, stagingSources.mipLevels);
    }

    stagingSources.staginBuffer = imageData.first;
    stagingSources.stagingBufferMemory = imageData.second;

    InitTexture(stagingSources);
}

void Texture::TransitionAndCopyImageBuffer(VkBuffer srcBuffer, std::optional<ktxTexture*> texture) const
{
    // Create Command Buffer
    CommandBuffer commandBuffer{};
    CommandBufferManager::CreateCommandBufferSingleUse(m_pContext, commandBuffer);
    std::vector<VkBufferImageCopy> bufferCopyRegions;

    uint32_t faces = m_TextureType == TextureType::TEXTURE_CUBE ? 6 : 1;
    for(uint32_t face{}; face < faces; ++face)
    {
        for (uint32_t mipLevel{}; mipLevel < m_MipLevels; ++mipLevel)
        {
            //Get the proper offset for the image
            ktx_size_t offset{};
            if(texture.has_value())
            {
                const auto returnCode = ktxTexture_GetImageOffset(texture.value(), mipLevel, 0, face, &offset);
                LogAssert(returnCode == KTX_SUCCESS, "Failed to get image offset",true );
            }


            VkBufferImageCopy region{};
            region.bufferOffset = offset;

            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = mipLevel;
            region.imageSubresource.baseArrayLayer = face;
            region.imageSubresource.layerCount = 1;

            region.imageExtent.width = m_ImageSize.x >> mipLevel;
            region.imageExtent.height = m_ImageSize.y >> mipLevel;
            region.imageExtent.depth = 1;

            bufferCopyRegions.push_back(region);
        }
    }


    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = m_MipLevels;
    subresourceRange.layerCount = static_cast<uint32_t>(bufferCopyRegions.size() / m_MipLevels);

    // Transition the image to transfer destination
    tools::InsertImageMemoryBarrier(commandBuffer.Handle, m_Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

    vkCmdCopyBufferToImage(commandBuffer.Handle, srcBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

    // Transition the image to shader read
    tools::InsertImageMemoryBarrier(commandBuffer.Handle, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);


    CommandBufferManager::EndCommandBufferSingleUse(m_pContext, commandBuffer);
}