#include "GWScene.hpp"

#include <iostream>

namespace GWIN
{
    GWScene::GWScene(GWModelLoader &modelLoader, JSONHandler &jsonHandler) : modelLoader(modelLoader), jsonHandler(jsonHandler)
    {
        std::shared_ptr<GWModel> model;

        modelLoader.importFile("C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/models/cube.obj", model, false);

        auto skybox = GWGameObject::createGameObject("Skybox");
        skybox.model = model;

        auto directionalLight = GWGameObject::createGameObject("Directional Light");
        directionalLight.transform.rotation.y = -.25 * glm::two_pi<float>();
        directionalLight.transform.rotation.x = .25 * glm::two_pi<float>();
        directionalLight.transform.rotation.z = .15 * glm::two_pi<float>();

        sceneInformation.gameObjects.emplace(skybox.getId(), std::move(skybox));
        sceneInformation.gameObjects.emplace(directionalLight.getId(), std::move(directionalLight));
    }

    GWScene::GWScene(std::string& sceneJson, GWModelLoader &modelLoader, JSONHandler &jsonHandler) : modelLoader(modelLoader), jsonHandler(jsonHandler)
    {

    }

    GWScene::~GWScene() {};

    void GWScene::createCamera()
    {
        GWCamera newCamera{};
        auto viewerObject = GWGameObject::createGameObject("Camera");
        
        newCamera.setViewerObject(viewerObject.getId());

        sceneInformation.gameObjects.emplace(viewerObject.getId(), std::move(viewerObject));
        sceneInformation.cameras.push_back(std::move(newCamera)); 
    }

    void GWScene::createGameObject(GameObjectInfo& objectInfo)
    {
        std::shared_ptr<GWModel> model;
        modelLoader.importFile(objectInfo.filePath, model, false);

        auto obj = GWGameObject::createGameObject(objectInfo.objName);
        obj.model = model;
        obj.textureDescriptorSet = objectInfo.texture;
        obj.transform.translation = objectInfo.position;
        obj.transform.scale = objectInfo.scale;

        sceneInformation.gameObjects.emplace(obj.getId(), std::move(obj)); 
    }

    void GWScene::removeGameObject(uint32_t id)
    {
        sceneInformation.gameObjects.erase(id);
    }

    void GWScene::saveScene(const std::string path)
    {
        nlohmann::json jsonObject;

        // Save all game objects
        for (const auto &gameObjectPair : sceneInformation.gameObjects)
        {
            jsonObject["gameObjects"].push_back(nlohmann::json::parse(gameObjectPair.second.toJson()));
        }

        for (const auto &camera : sceneInformation.cameras)
        {
            jsonObject["cameras"].push_back(nlohmann::json::parse(camera.toJson()));
        }

        jsonHandler.setValue(sceneInformation.name, jsonObject);
        jsonHandler.saveToFile(path, sceneInformation.name); 
    }
}