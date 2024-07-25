#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "GWGameObject.hpp"
#include "GWModel.hpp"

#include <string>
#include <vector>
#include <cassert>

namespace GWIN
{
    class GWModelLoader
    {
    public:
        GWModelLoader(GWinDevice& device) : device(device) {};
        bool importFile(const std::string &pfile, std::shared_ptr<GWModel> &model, bool isMultipleModels);

    private:
        Assimp::Importer importer;
        GWinDevice& device;

        std::vector<std::shared_ptr<GWModel>> processScene(const aiScene *scene);
        std::shared_ptr<GWModel> processMesh(aiMesh *mesh);
    };
}