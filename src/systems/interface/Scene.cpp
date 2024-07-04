#include "Scene.hpp"

namespace GWIN
{
    GWScene::GWScene()
    {

    }

    GWScene::~GWScene()
    {

    }

    void GWScene::renderScene(VkImageView& imageView)
    {
        if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoBackground))
        {
            ImTextureID myTextureID = (ImTextureID)imageView;
            ImGui::Image(myTextureID, ImVec2(512, 512));
        } ImGui::End();
    }
}