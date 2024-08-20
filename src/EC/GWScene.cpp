#include "GWScene.hpp"

#include <iostream>

namespace GWIN
{
    GWScene::GWScene(SceneCreateInfo createInfo)
        : modelLoader(createInfo.modelLoader),
          jsonHandler(createInfo.jsonHandler),
          name(createInfo.name),
          textureLayout(createInfo.textureLayout),
          texturePool(createInfo.texturePool)
    {
        if (createInfo.sceneJson == "")
        {
            std::shared_ptr<GWModel> model;

            modelLoader.importFile("C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/models/cube.obj", model, false);

            auto skybox = GWGameObject::createGameObject("Skybox");
            skybox.model = model;

            auto directionalLight = GWGameObject::createGameObject("Directional Light");
            directionalLight.transform.rotation.y = -.25f * glm::two_pi<float>();
            directionalLight.transform.rotation.x = .25f * glm::two_pi<float>();
            directionalLight.transform.rotation.z = .15f * glm::two_pi<float>();

            gameObjects.emplace(skybox.getId(), std::move(skybox));
            gameObjects.emplace(directionalLight.getId(), std::move(directionalLight));
        } else {
            loadScene(createInfo.sceneJson);
        }
    }

    void GWScene::loadScene(const std::string& sceneJson)
    {
        try
        {
            nlohmann::json jsonData = nlohmann::json::parse(sceneJson);

            if (jsonData.contains("gameObjects"))
            {
                for (const auto &obj : jsonData["gameObjects"])
                {
                    float translation[3]{obj["transform"]["translation"].get<float>()};
                    float rotation[3]{obj["transform"]["rotation"].get<float>()};
                    uint32_t textures[6]{obj["textures"].get<uint32_t>()};

                    auto gameObject = GWGameObject::createGameObject(obj["name"].get<std::string>());
                    gameObject.transform.translation = {translation[0], translation[1], translation[2]};
                    gameObject.transform.rotation = {rotation[0], rotation[1], rotation[2]};
                    gameObject.transform.scale = obj["transform"]["scale"].get<float>();
                    
                    for (uint32_t i = 0; i < 6; ++i)
                    {
                        gameObject.Textures[i] = textures[i];
                    }

                    if (obj.contains("modelPath"))
                    {
                        std::shared_ptr<GWModel> model;
                        modelLoader.importFile(obj["modelPath"].get<std::string>(), model, false);
                        gameObject.model = model;
                    }

                    gameObjects.emplace(gameObject.getId(), std::move(gameObject));
                }
            }

            if (jsonData.contains("cameras"))
            {
                for (const auto& cam : jsonData["cameras"])
                {
                    GWCamera camera;
                    auto viewerObject = GWGameObject::createGameObject("Camera");
                    camera.setViewerObject(viewerObject.getId());
                    camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

                    cameras.push_back(std::move(camera));
                    gameObjects.emplace(viewerObject.getId(), std::move(viewerObject));
                }
            }
        }
        catch (const nlohmann::json::parse_error& e)
        {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    }

    GWScene::~GWScene() {};

    void GWScene::createSet(VkDescriptorSet &set, Texture &texture)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = texture.textureImage.layout;
        imageInfo.imageView = texture.textureImage.imageView;
        imageInfo.sampler = texture.textureSampler;

        GWDescriptorWriter(*textureLayout, *texturePool)
            .writeImage(0, &imageInfo)
            .build(set);

        textures.push_back(std::move(set));
    }

    void GWScene::createCamera()
    {
        GWCamera newCamera{};
        auto viewerObject = GWGameObject::createGameObject("Camera");
        
        newCamera.setViewerObject(viewerObject.getId());

        gameObjects.emplace(viewerObject.getId(), std::move(viewerObject));
        cameras.push_back(std::move(newCamera)); 
    }

    void GWScene::createGameObject(GameObjectInfo& objectInfo)
    {
        std::shared_ptr<GWModel> model;
        modelLoader.importFile(objectInfo.filePath, model, false);

        auto obj = GWGameObject::createGameObject(objectInfo.objName);
        obj.model = model;
        obj.Textures[objectInfo.textureType] = objectInfo.texture;
        obj.transform.translation = objectInfo.position;
        obj.transform.scale = objectInfo.scale;

        gameObjects.emplace(obj.getId(), std::move(obj)); 
    }

    void GWScene::removeGameObject(uint32_t id)
    {
        gameObjects.erase(id);
    }

    void GWScene::saveScene(const std::string path)
    {
        nlohmann::json jsonObject;

        jsonObject["sceneName"] = name;

        for (const auto &gameObjectPair : gameObjects)
        {
            jsonObject["gameObjects"].push_back(nlohmann::json::parse(gameObjectPair.second.toJson()));
        }

        for (const auto &camera : cameras)
        {
            jsonObject["cameras"].push_back(nlohmann::json::parse(camera.toJson()));
        }

        jsonHandler.setValue(name, jsonObject);
        jsonHandler.saveToFile(path, name); 
    }
}