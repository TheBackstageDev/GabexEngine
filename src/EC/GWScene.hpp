#pragma once

#include "GWFrameInfo.hpp"
#include "GWModelLoader.hpp"
#include "GWTextureHandler.hpp"
#include "../JSONHandler.hpp"

namespace GWIN
{
    class GWScene
    {
    public:
        GWScene(GWModelLoader& modelLoader, JSONHandler& jsonHandler);
        GWScene(std::string& sceneJson, GWModelLoader& modelLoader, JSONHandler& jsonHandler);

        GWScene(const GWScene &) = delete;
        GWScene &operator=(const GWScene &) = delete;

        ~GWScene();

        void createCamera();
        void createGameObject(GameObjectInfo &objectInfo);
        void createGameObject(GWGameObject& obj) { sceneInformation.gameObjects.emplace(obj.getId(), std::move(obj)); }

        void removeGameObject(uint32_t id);

        void saveScene(const std::string path);
        GWCamera& getCurrentCamera() { return sceneInformation.cameras.at(currentCamera); }

        SceneInfo &getSceneInfo() { return sceneInformation; }

    private:
        uint32_t currentCamera{0};

        SceneInfo sceneInformation;

        JSONHandler& jsonHandler;
        GWModelLoader& modelLoader;
    };
}