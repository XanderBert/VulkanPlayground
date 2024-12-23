//
// Created by berte on 4/29/2024.
//

#include "SceneManager.h"

void SceneManager::RenderPresent(VkCommandBuffer commandBuffer)
{
	m_ActiveScene->AlbedoRender(commandBuffer);
}

void SceneManager::Render(VkCommandBuffer commandBuffer) {
    m_ActiveScene->Render(commandBuffer);
}

void SceneManager::RenderDepth(VkCommandBuffer commandBuffer)
{
    m_ActiveScene->RenderDepth(commandBuffer);
}


void SceneManager::ExecuteComputePass(VkCommandBuffer commandBuffer){
    m_ActiveScene->ExecuteComputePass(commandBuffer);
}

void SceneManager::AddScene(std::unique_ptr<Scene> scene) {
    m_Scenes.emplace_back(std::move(scene));
    if (m_ActiveScene == nullptr)
        m_ActiveScene = m_Scenes.back().get();
}
Scene *SceneManager::GetActiveScene() { return m_ActiveScene; }
void SceneManager::CleanUp() {
    for(auto& scene: m_Scenes) {
        scene->CleanUp();
    }
}
