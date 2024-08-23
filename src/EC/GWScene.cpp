#include "GWScene.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace GWIN
{
    GWScene::GWScene(SceneCreateInfo createInfo)
        : device(createInfo.device),
          modelLoader(createInfo.modelLoader),
          jsonHandler(createInfo.jsonHandler),
          textureHandler(createInfo.textureHandler),
          materialHandler(createInfo.materialHandler),
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
                gameObjects.clear();
                std::shared_ptr<GWModel> model;
                for (const auto &obj : jsonData["gameObjects"])
                {
                    auto gameObject = GWGameObject::createGameObject(obj["name"].get<std::string>(), obj["id"].get<uint32_t>());
                    
                    glm::vec3 translation = {
                        obj["transform"]["translation"][0].get<float>(),
                        obj["transform"]["translation"][1].get<float>(),
                        obj["transform"]["translation"][2].get<float>()};

                    glm::vec3 rotation = {
                        obj["transform"]["rotation"][0].get<float>(),
                        obj["transform"]["rotation"][1].get<float>(),
                        obj["transform"]["rotation"][2].get<float>()};

                    gameObject.transform.scale = obj["transform"]["scale"].get<float>();
                    gameObject.transform.rotation = rotation;
                    gameObject.transform.translation = translation;

                    if (obj.contains("textures"))
                    {
                        auto textures = obj["textures"].get<std::vector<uint32_t>>();
                        for (size_t i = 0; i < textures.size(); ++i)
                        {
                            gameObject.Textures[i] = textures[i];
                        }
                    }

                    if (obj.contains("material"))
                    {
                        gameObject.Material = obj["material"].get<uint32_t>();
                    }

                    if (obj.contains("modelPath"))
                    {
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

                    camera.setViewerObject(cam["viewerobject"]);

                    auto& viewerObject = gameObjects.at(camera.getViewerObject());
                    camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

                    cameras.emplace(viewerObject.getId(), camera);
                } 
            }

            if (jsonData.contains("texturesinfo"))
            {
                texturePool->resetPool();
                textures.clear();

                for (const auto &textureData : jsonData["texturesinfo"])
                {
                    Texture texture = textureHandler->createTexture(textureData["path"].get<std::string>(), true);

                    VkDescriptorSet newSet;
                    createSet(newSet, texture);
                    textures.push_back(std::move(newSet));
                }
            }

            if (jsonData.contains("materials"))
            {
                
            }
        }
        catch (const nlohmann::json::parse_error& e)
        {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }

        std::cout << "Completed loading! \n";
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
        cameras.emplace(newCamera.getId(), newCamera);
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

     void GWScene::createGameObject(GWGameObject& obj)
     {
        gameObjects.emplace(obj.getId(), std::move(obj)); 
     }

    void GWScene::removeGameObject(uint32_t id)
    {
        gameObjects.erase(id);
    }

    //helper Func
    std::string formatFloat(float value, int precision)
    {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(precision) << value;
        return stream.str();
    }
    void GWScene::saveScene(const std::string path)
    {
        nlohmann::json jsonObject;

        jsonObject["scene"]["sceneName"] = name;
        jsonObject["scene"]["skybox"] = 1;

        for (const auto &gameObjectPair : gameObjects)
        {
            jsonObject["gameObjects"].push_back(nlohmann::json::parse(gameObjectPair.second.toJson()));
        }

        for (const auto &camera : cameras)
        {
            jsonObject["cameras"].push_back(nlohmann::json::parse(camera.second.toJson()));
        }

        auto& texturesToSerialize = textureHandler->getTextures();

        for (const auto& texture : texturesToSerialize)
        {
            nlohmann::json textureObject;
            textureObject["id"] = texture.id;
            textureObject["path"] = texture.pathToTexture;

            jsonObject["texturesinfo"].push_back(nlohmann::json::parse(textureObject.dump()));
        }

        auto& materialsToSerialize = materialHandler->getMaterials();

        for (const auto &material : materialsToSerialize)
        {
            if (material.color.x == -431602080 && material.color.y == -431602080 && material.color.z == -431602080)
                continue;
                
            nlohmann::json materialObject;
            materialObject["color"] = {
                formatFloat(material.color.r, 2),
                formatFloat(material.color.g, 2),
                formatFloat(material.color.b, 2),
                formatFloat(material.color.a, 2)};
            materialObject["data"] = {
                formatFloat(material.data.x, 2),
                formatFloat(material.data.y, 2),
                formatFloat(material.data.z, 2)};

            jsonObject["materials"].push_back(materialObject);
        }

        jsonHandler.setValue(name, jsonObject);
        jsonHandler.saveToFile(path, name); 
    }
}