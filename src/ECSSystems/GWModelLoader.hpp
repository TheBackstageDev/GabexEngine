#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "GWGameObject.hpp"

#include <optional>
#include <string>
#include <vector>

namespace GWIN
{
    class GWModelLoader
    {
    public:
        bool importFile(const std::string &pFile, const GWGameObject& gameObject, const std::optional<std::vector<GWGameObject>>& gameObjects);
    private:
        Assimp::Importer importer;

        std::vector<GWModel> processScene(aiScene *scene);
        GWModel processMesh(aiMesh *mesh);
    };
}