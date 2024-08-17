#include "GWScene.hpp"

namespace GWIN
{
    GWScene::GWScene(GWModelLoader &modelLoader) : modelLoader(modelLoader)
    {
        std::shared_ptr<GWModel> model;

        modelLoader.importFile("C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/models/cube.obj", model, false);

        auto skybox = GWGameObject::createGameObject("Skybox");
        skybox.model = model;

        auto directionalLight = GWGameObject::createGameObject("Directional Light");

        sceneInformation.gameObjects.emplace(skybox.getId(), std::move(skybox));
        sceneInformation.gameObjects.emplace(directionalLight.getId(), std::move(directionalLight));
    }

    GWScene::GWScene(SceneInfo &info, GWModelLoader &modelLoader) : modelLoader(modelLoader)
    {
/*         sceneInformation.gameObjects = info.gameObjects;
        sceneInformation.cameras = info.cameras; */
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

    void GWScene::saveScene()
    {

    }
}