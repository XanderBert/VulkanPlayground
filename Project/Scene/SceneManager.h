#pragma once
#include <memory>
#include <vector>

#include "Scene.h"


class SceneManager
{
public:
    SceneManager() = default;
    ~SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
    SceneManager(SceneManager&&) = delete;
    SceneManager& operator=(SceneManager&&) = delete;

    static void Render(VkCommandBuffer commandBuffer);
    static void AddScene(std::unique_ptr<Scene> scene);
    static Scene *GetActiveScene();
    static void CleanUp();

private:
    inline static Scene* m_ActiveScene{};
    inline static std::vector<std::unique_ptr<Scene>> m_Scenes{};
};

