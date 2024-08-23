#pragma once

#include "GWFrameInfo.hpp"
#include "GWModelLoader.hpp"
#include "GWTextureHandler.hpp"
#include "GWMaterialHandler.hpp"
#include "../JSONHandler.hpp"

#include <array>

namespace GWIN
{
    struct SceneCreateInfo
    {
        SceneCreateInfo(
            GWinDevice& device,
            std::unique_ptr<GWDescriptorSetLayout>& textureLayout, 
            std::unique_ptr<GWDescriptorPool>& texturePool,
            GWModelLoader& modelLoader, 
            JSONHandler& jsonHandler, 
            std::unique_ptr<GWTextureHandler>& textureHandler,
            std::unique_ptr<GWMaterialHandler>& materialHandler,
            std::string name = "DefaultName", 
            std::string sceneJson = ""
        ) : 
            device(device),
            textureLayout(textureLayout), 
            texturePool(texturePool),
            modelLoader(modelLoader), 
            jsonHandler(jsonHandler), 
            textureHandler(textureHandler),
            materialHandler(materialHandler),
            name(name), 
            sceneJson(sceneJson) 
        {}

        GWinDevice& device;
        std::unique_ptr<GWDescriptorSetLayout>& textureLayout;
        std::unique_ptr<GWDescriptorPool>& texturePool;
        GWModelLoader& modelLoader;
        std::unique_ptr<GWTextureHandler>& textureHandler;
        std::unique_ptr<GWMaterialHandler>& materialHandler;
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
        void createGameObject(GWGameObject& obj);

        void removeGameObject(uint32_t id);
        void createSet(VkDescriptorSet &set, Texture &texture);

        void saveScene(const std::string path);
        GWCamera& getCurrentCamera() { return cameras.at(currentCamera); }

        void setNewCamera(uint32_t newCamera) { currentCamera = newCamera; }
        void loadScene(const std::string &sceneJson);

        GWGameObject::map& getGameObjects() { return gameObjects; }
        std::vector<VkDescriptorSet>& getTextures() { return textures; }

    private:
        uint32_t currentCamera{0};

        std::string name = "DefaultScene";
        GWGameObject::map gameObjects;
        std::vector<VkDescriptorSet> textures;
        std::unordered_map<uint32_t, GWCamera> cameras;

        GWinDevice &device;

        std::unique_ptr<GWIN::GWDescriptorSetLayout>& textureLayout;
        std::unique_ptr<GWDescriptorPool>& texturePool;

        std::unique_ptr<GWMaterialHandler>& materialHandler;
        std::unique_ptr<GWTextureHandler>& textureHandler;        
        JSONHandler& jsonHandler;
        GWModelLoader& modelLoader;
    };
}