#include "GWModelLoader.hpp"

#include <optional>
#include <stdexcept>
#include <iostream>

namespace GWIN
{
    bool GWModelLoader::importFile(const std::string& pfile, const GWGameObject& gameObject, const std::optional<std::vector<GWGameObject>>& gameObjects)
    {
        const aiScene *scene = importer.ReadFile(pfile,
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate);

        if (nullptr == scene)
        {
            std::runtime_error(importer.GetErrorString());
            return false;
        }

        if (!gameObjects.has_value() || gameObjects->empty())
        {

        } else {

        }

        return true;
    }

    std::vector<GWModel> GWModelLoader::processScene(aiScene* scene)
    {
        if (!scene->HasMeshes())
        {
            std::cerr << "Scene has no meshes to process" << std::endl;
            return;
        }

        std::vector<GWModel> processedModels{scene->mNumMeshes};

        for (unsigned short i = 0; i < scene->mNumMeshes; ++i)
        {
            processedModels.push_back(std::move(processMesh(scene->mMeshes[i])));
        }

        return processedModels;
    }

    GWModel GWModelLoader::processMesh(aiMesh* mesh)
    {

    }
}