#pragma once

#include "GWFrameInfo.hpp"
#include "GWModelLoader.hpp"
#include "GWTextureHandler.hpp"
#include "../JSONHandler.hpp"

namespace GWIN
{
    struct SceneCreateInfo
    {
        SceneCreateInfo(
            std::unique_ptr<GWDescriptorSetLayout>& textureLayout, 
            std::unique_ptr<GWDescriptorPool>& texturePool,
            GWModelLoader& modelLoader, 
            JSONHandler& jsonHandler, 
            std::string name = "DefaultName", 
            std::string sceneJson = ""
        ) : 
            textureLayout(textureLayout), 
            texturePool(texturePool),
            modelLoader(modelLoader), 
            jsonHandler(jsonHandler), 
            name(name), 
            sceneJson(sceneJson) 
        {}

        std::unique_ptr<GWDescriptorSetLayout>& textureLayout;
        std::unique_ptr<GWDescriptorPool>& texturePool;
        GWModelLoader& modelLoader;
        JSONHandler& jsonHandler;
        std::string name;
        std::string sceneJson;
    };

    class GWScene
    {
    public:
        GWScene(SceneCreateInfo createInfo);

        GWScene(const GWScene &) = delete;
        GWScene &operator=(const GWScene &) = delete;

        ~GWScene();

        void createCamera();
        void createGameObject(GameObjectInfo &objectInfo);
        void createGameObject(GWGameObject& obj) { gameObjects.emplace(obj.getId(), std::move(obj)); }

        void removeGameObject(uint32_t id);
        void createSet(VkDescriptorSet &set, Texture &texture);

        void saveScene(const std::string path);
        GWCamera& getCurrentCamera() { return cameras.at(currentCamera); }

        GWGameObject::map& getGameObjects() { return gameObjects; }
        std::vector<VkDescriptorSet>& getTextures() { return textures; }

    private:
        void loadScene(const std::string &sceneJson);

        uint32_t currentCamera{0};

        std::string name = "DefaultScene";
        GWGameObject::map gameObjects;
        std::vector<VkDescriptorSet> textures;
        std::vector<GWCamera> cameras;

        std::unique_ptr<GWIN::GWDescriptorSetLayout>& textureLayout;
        std::unique_ptr<GWDescriptorPool>& texturePool;

        JSONHandler& jsonHandler;
        GWModelLoader& modelLoader;
    };
}