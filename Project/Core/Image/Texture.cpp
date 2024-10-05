#include "Texture.h"

#include <filesystem>
#include <utility>

#include "vulkanbase/VulkanTypes.h"
#include "Core/CommandBuffer.h"
#include "ImGuiFileDialog.h"
#include "Patterns/ServiceLocator.h"
#include "vulkanbase/VulkanUtil.h"


Texture::Texture(const std::variant<std::filesystem::path,ImageInMemory>& pathOrImage, VulkanContext *vulkanContext, ColorType colorType, TextureType textureType)
: m_pContext(vulkanContext)
, m_ColorType(colorType)
, m_TextureType(textureType)
{
    std::visit([this](auto&& arg){ this->InitTexture(arg);}, pathOrImage);
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

    //Cleanup the image and the memory
    std::visit([this](auto&& arg) { this->CleanupImage(arg, this->m_Image); }, m_ImageMemory);

    vkDestroySampler(device, m_Sampler, nullptr);
    vkDestroyImageView(device, m_ImageView, nullptr);
}

void Texture::InitTexture(const TextureData &loadedImage)
{
    //Check if the variant is a ktxVulkanTexture
    if(std::holds_alternative<ktxVulkanTexture>(loadedImage))
    {
        const auto& kTexture = std::get<ktxVulkanTexture>(loadedImage);
        m_Image = kTexture.image;
        m_ImageMemory = kTexture.deviceMemory;
        m_ImageSize = {kTexture.width, kTexture.height};
        m_MipLevels = kTexture.levelCount;

        //TODO: Figure out how i can use ktx to make VK_FORMAT_R8G8B8A8_SRGB images
        Image::CreateImageView(m_pContext->device, m_Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT,m_ImageView, m_TextureType);
    }else
    {
        const auto &imageInMemory = std::get<ImageInMemory>(loadedImage);
        m_ImageSize = imageInMemory.imageSize;
        m_MipLevels = imageInMemory.mipLevels;

        VmaAllocation allocation{};
        Image::CreateImage(m_ImageSize.x, m_ImageSize.y, m_MipLevels, VK_SAMPLE_COUNT_1_BIT, static_cast<VkFormat>(m_ColorType), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, m_Image, allocation, m_TextureType);
        m_ImageMemory = allocation;

        TransitionAndCopyImageBuffer(imageInMemory.staginBuffer);
        vmaDestroyBuffer(Allocator::VmaAllocator, imageInMemory.staginBuffer, imageInMemory.stagingBufferMemory);

        Image::CreateImageView(m_pContext->device, m_Image, static_cast<VkFormat>(m_ColorType), VK_IMAGE_ASPECT_COLOR_BIT,m_ImageView, m_TextureType);
    }

    Image::CreateSampler(m_pContext, m_Sampler, m_MipLevels);


    //Setup the ImGuiTexture
    if(m_TextureType == TextureType::TEXTURE_CUBE) return;
    m_ImGuiTexture = std::make_unique<ImGuiTexture>(m_Sampler, m_ImageView, ImVec2(250, 250));
}

void Texture::InitTexture(const std::filesystem::path &path)
{
    m_Path = VulkanContext::GetAssetPath() / path;

    TextureData textureData;


    //Note: For now we only support .ktx files for a cubemap
    if (m_Path.value().extension() == ".ktx" || m_TextureType == TextureType::TEXTURE_CUBE)
    {
        textureData = ktx::CreateImage(m_Path.value());
    }
    else
    {
        ImageInMemory stagingSources{};
        auto bufferPair = stbi::CreateImage(m_Path.value(), stagingSources.imageSize, stagingSources.mipLevels);
        stagingSources.staginBuffer = bufferPair.first;
        stagingSources.stagingBufferMemory = bufferPair.second;

        textureData = stagingSources;
    }

    InitTexture(textureData);
}

void Texture::TransitionAndCopyImageBuffer(VkBuffer srcBuffer) const
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
            VkDeviceSize offset = 0;
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

void Texture::CleanupImage(VkDeviceMemory deviceMemory, VkImage image)
{
    //todo: fix this uglyness with the service locator
    const auto device = ServiceLocator::GetService<VulkanContext>()->device;
    vkFreeMemory(device, deviceMemory, nullptr);
    vkDestroyImage(device, image, nullptr);
}

void Texture::CleanupImage(VmaAllocation deviceMemory, VkImage image)
{
    vmaDestroyImage(Allocator::VmaAllocator, image, deviceMemory);
}
