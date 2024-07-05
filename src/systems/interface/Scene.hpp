#pragma once

#include <imgui/imgui.h>
#include "GWGameObject.hpp"

namespace GWIN
{
    class GWScene
    {
    public:
        GWScene();
        ~GWScene();

        void renderScene(VkImageView& imageView);
        void addGameObject();

        GWGameObject::map getGameObjects() const { return gameObjects; }
    private:
        GWGameObject::map gameObjects;
    };
}