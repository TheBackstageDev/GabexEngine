#include "GWScene.hpp"

#include <iostream>
#include "../systems/interface/Console.hpp"

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
            auto skybox = GWGameObject::createGameObject("Skybox");
            skybox.model = createMesh("src/models/cube.obj", std::nullopt, std::nullopt);

            auto directionalLight = GWGameObject::createGameObject("Directional Light");
            directionalLight.transform.rotateEuler({45.f, 45.f, 45.f});

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

                    gameObject.transform.scale = {
                        obj["transform"]["scale"][0].get<float>(),
                        obj["transform"]["scale"][1].get<float>(),
                        obj["transform"]["scale"][2].get<float>()
                    };
                    gameObject.transform.rotation = rotation;
                    gameObject.transform.translation = translation;

                    if (obj.contains("model"))
                    {
                        gameObject.model = obj["model"].get<uint32_t>();
                    }

                    if (obj.contains("light"))
                    {
                        auto light = std::make_unique<LightComponent>();
                        light->lightIntensity = obj["light"]["lightIntensity"].get<float>();
                        light->cutOffAngle = obj["light"]["cutOffAngle"].get<float>();
                        gameObject.light = std::move(light);
                    }
                    
                    gameObjects.emplace(gameObject.getId(), std::move(gameObject));
                } 
            }

            if (jsonData.contains("meshes"))
            {
                for (const auto& mesh : jsonData["meshes"])
                {
                    createMesh(mesh["path"].get<std::string>(), std::nullopt);
                }
            }

            if (jsonData.contains("cameras"))
            {
                for (const auto& cam : jsonData["cameras"])
                {
                    GWCamera camera;

                    camera.setViewerObject(cam["viewerobject"]);

                    auto& viewerObject = gameObjects.at(camera.getViewerObject());
                    camera.setViewYXZ(viewerObject.transform.translation, glm::eulerAngles(viewerObject.transform.rotation));

                    cameras.emplace(viewerObject.getId(), camera);
                } 
            }

            if (jsonData.contains("texturesinfo"))
            {
                texturePool->resetPool();
                textureHandler->resetTextures();

                for (const auto &textureData : jsonData["texturesinfo"])
                {
                    Texture texture = textureHandler->createTexture(textureData["path"].get<std::string>(), true);

                    createSet(texture);
                }
            }

            if (jsonData.contains("materials"))
            {
                materialHandler->resetMaterials();

                for (const auto &material : jsonData["materials"])
                {
                    glm::vec4 color;
                    for (size_t i = 0; i < 4; ++i)
                    {
                        color[i] = material["color"][i].get<float>();
                    }

                    float roughness = material["data"][0].get<float>();
                    float metallic =(material["data"][1].get<float>());

                    materialHandler->createMaterial(roughness, metallic, color, material["name"].get<std::string>());
                }
            }
        }
        catch (const nlohmann::json::parse_error& e)
        {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    }

    GWScene::~GWScene() {};

    void GWScene::createSet(Texture &texture, bool replace)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = texture.textureImage.layout;
        imageInfo.imageView = texture.textureImage.imageView;
        imageInfo.sampler = texture.textureSampler;

        GWDescriptorWriter(*textureLayout, *texturePool)
            .writeImage(0, &imageInfo, texture.id)
            .build(this->textures, true);
    }

    void GWScene::retcreateSet(VkImageLayout layout, VkImageView &imageView, VkSampler &sampler, uint32_t binding)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = layout;
        imageInfo.imageView = imageView;
        imageInfo.sampler = sampler;

        GWDescriptorWriter(*textureLayout, *texturePool)
            .writeImage(binding, &imageInfo)
            .build(this->textures);
    }

    void GWScene::createCamera()
    {
        GWCamera newCamera{};
        auto viewerObject = GWGameObject::createGameObject("Camera");
        
        newCamera.setViewerObject(viewerObject.getId());

        gameObjects.emplace(viewerObject.getId(), std::move(viewerObject));
        cameras.emplace(newCamera.getId(), newCamera);
    }

    void GWScene::createGameObject(GameObjectType type)
    {
        auto &camera = gameObjects.at(cameras.at(currentCamera).getViewerObject());

        const glm::vec3 dPos = camera.transform.translation + glm::vec3(-.5f, 1.f, -.5f);
        GWGameObject obj;

        switch (type)
        {
        case GameObjectType::BasicObject:
        {
            obj = GWGameObject::createGameObject("New Object");
            break;
        }
        case GameObjectType::PointLight:
        {
            obj = GWGameObject::createLight(1.f, 0.1f, glm::vec3(1.0f, 1.0f, 1.0f));
            break;
        }
        case GameObjectType::SpotLight:
        {
            obj = GWGameObject::createLight(1.f, 0.1f, glm::vec3(1.0f, 1.0f, 1.0f), 35.f);
            break;
        }
        case GameObjectType::Camera:
        {
            createCamera();
            break;
        }
        default:
            throw std::invalid_argument("Unknown GameObjectType");
        }

        obj.transform.translation = dPos;

        createGameObject(obj);
    }

     void GWScene::createGameObject(GWGameObject& obj)
     {
        gameObjects.emplace(obj.getId(), std::move(obj)); 
     }

    void GWScene::removeGameObject(uint32_t id)
    {
        gameObjects.erase(id);
    }

    uint32_t GWScene::createMesh(const std::string &pathToFile, std::optional<uint32_t> replaceId, std::optional<std::string> info)
    {
        modelLoader.importFile(pathToFile, model);

        if (replaceId.has_value())
        {
            meshes[replaceId.value()] = std::move(model);

            return replaceId.value();
        }
        else
        {
            uint32_t newMeshID = ++lastMeshID;

            if (info.has_value())
            {
                nlohmann::json jsonData = nlohmann::json::parse(info.value());

                if (jsonData.contains("material"))
                {
                    uint32_t materialId = jsonData["material"].get<uint32_t>();
                    model->Material = materialId;
                }
            }

            meshes[newMeshID] = std::move(model);

            return lastMeshID;
        }
    }

    void GWScene::removeMesh(uint32_t id)
    {
        meshes.erase(id);
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

        for (const auto& mesh : meshes)
        {
            jsonObject["meshes"].push_back(nlohmann::json::parse(mesh.second->toJson()));
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
        auto& materialsDataToSerialize = materialHandler->getMaterialData();

        for (size_t i = 0; i < materialsToSerialize.size(); ++i)
        {
            const auto &material = materialsToSerialize[i];

            // Skip the material if the color is null
            if (material.color.x == -431602080 && material.color.y == -431602080 && material.color.z == -431602080)
                continue;

            nlohmann::json materialObject;
            materialObject["color"] = {
                material.color.r,
                material.color.g,
                material.color.b,
                material.color.a};
            materialObject["data"] = {
                material.data.x,
                material.data.y,
                material.data.z};

            materialObject["name"] = materialsDataToSerialize[i].name;

            jsonObject["materials"].push_back(nlohmann::json::parse(materialObject.dump()));
        }

        jsonHandler.setValue(name, jsonObject);
        jsonHandler.saveToFile(path, name); 

        GWConsole::addLog("Scene sucessfully saved!");
    }
}