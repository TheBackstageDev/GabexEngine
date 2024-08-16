#pragma once

#include "GWFrameInfo.hpp"

namespace GWIN
{
    class GWScene
    {
    public:
        GWScene();
        GWScene(SceneInfo& info);

        GWScene(const GWScene &) = delete;
        GWScene &operator=(const GWScene &) = delete;

        ~GWScene();

        void createGameObject();
        void removeGameObject(uint32_t id);

        void saveScene();

        SceneInfo &getSceneInfo() { return sceneInformation; }

    private:
        SceneInfo sceneInformation;
    };
}