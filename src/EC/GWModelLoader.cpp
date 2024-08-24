#include "GWModelLoader.hpp"

#include <iostream>

namespace GWIN
{
    bool GWModelLoader::importFile(const std::string& pfile, std::shared_ptr<GWModel>& model)
    {
        const aiScene *scene = importer.ReadFile(pfile,
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate);

        if (nullptr == scene)
        {
            std::cout << importer.GetErrorString();
            return false;
        }

        std::vector<std::shared_ptr<GWModel>> objects = processScene(scene);

        if (objects.size() > 1)
        {
            model = objects.at(0); 
            model->setPath(pfile);

            objects.erase(objects.begin());

            for (auto &subModel : objects)
            {
                model->addSubModel(subModel);
            }
        } else {
            model = objects[0];
            model->setPath(pfile);
        }

        return true;
    }

    std::vector<std::shared_ptr<GWModel>> GWModelLoader::processScene(const aiScene* scene)
    {
        assert(scene->HasMeshes() && "Scene needs Meshes!");

        std::vector<std::shared_ptr<GWModel>> processedModels;

        for (unsigned short i = 0; i < scene->mNumMeshes; ++i)
        {
            processedModels.push_back(processMesh(scene->mMeshes[i]));
        }

        return processedModels;
    }

    std::shared_ptr<GWModel> GWModelLoader::processMesh(aiMesh* mesh)
    {
        assert(mesh->mNumVertices >= 3 && "Mesh needs atleast 3 Vertices!");

        //Processes Vertices
        std::vector<GWModel::Vertex> vertices{mesh->mNumVertices};
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {
            glm::vec2 vertexUv = { 0.0f, 0.0f };
            glm::vec3 vertexTangent = {0.0f, 0.0f, 0.0f};
            if (mesh->mTextureCoords[0]) 
            {
                vertexUv.x = mesh->mTextureCoords[0][i].x;
                vertexUv.y = 1 - mesh->mTextureCoords[0][i].y;
            }

            GWModel::Vertex vertex;
            vertex.position = {mesh->mVertices[i].x, -mesh->mVertices[i].y, mesh->mVertices[i].z};
            if (mesh->HasVertexColors(i))
            {
                glm::vec3 vertexColor = {mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b};
                vertex.color = vertexColor;
            } else {
                vertex.color = {.5f, .5f, .5f};
            }
            if (mesh->HasNormals())
            {
                glm::vec3 vertexNormal = {mesh->mNormals[i].x, -mesh->mNormals[i].y, mesh->mNormals[i].z};
                vertex.normal = vertexNormal;
            }
            if (mesh->HasTangentsAndBitangents())
            {
                vertexTangent = {mesh->mTangents[i].x, -mesh->mTangents->y, mesh->mTangents->z};
            }
            vertex.uv = vertexUv;
            vertex.tangent = vertexTangent;

            vertices[i] = vertex;
        }

        //Processes Indices
        std::vector<unsigned int> indices;
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            const aiFace& Face = mesh->mFaces[i];
            if(Face.mNumIndices == 3) {
                indices.push_back(Face.mIndices[0]);
                indices.push_back(Face.mIndices[1]);
                indices.push_back(Face.mIndices[2]);
            }
        }

        const GWModel::Builder builder{vertices, indices};

        std::shared_ptr<GWModel> model;
        model = std::make_shared<GWModel>(device, builder);

        return model;
    }
}