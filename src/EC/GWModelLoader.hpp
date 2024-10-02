#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "GWGameObject.hpp"
#include "GWModel.hpp"
#include "GWTextureHandler.hpp"

#include <string>
#include <vector>
#include <cassert>

namespace GWIN
{
    class GWModelLoader
    {
    public:
        GWModelLoader(GWinDevice& device, std::unique_ptr<GWTextureHandler>& textureHandler) : device(device), textureHandler(textureHandler) {};
        bool importFile(const std::string &pfile, std::shared_ptr<GWModel> &model);

        void setCreateTextureCallback(std::function<void(Texture &texture)> callback)
        {
            this->createTextureCallback = callback;
        }

    private:
        Assimp::Importer importer;
        GWinDevice& device;

        std::vector<std::shared_ptr<GWModel>> processScene(const aiScene *scene, const std::string &pfile);
        std::shared_ptr<GWModel> processMesh(aiMesh *mesh, aiMaterial *material, const std::string &pfile);
        void processMaterialTextures(std::shared_ptr<GWModel>& model, aiMaterial* material, const std::string& pfile);

        std::function<void(Texture &texture)> createTextureCallback;
        std::unique_ptr<GWTextureHandler>& textureHandler;
    };
}