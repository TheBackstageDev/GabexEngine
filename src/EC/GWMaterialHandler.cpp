#include "GWMaterialHandler.hpp"

#include <iostream>

namespace GWIN
{
    GWMaterialHandler::GWMaterialHandler(GWinDevice& device) : device(device)
    {
        createMaterial(0.5f, .5f, {1.0f, 1.0f, 1.0f, 1.0f}, "Default Material"); //Default material
    }

    void GWMaterialHandler::setMaterials(MaterialBuffer& ubo)
    {
        for (uint32_t i = 0; i < lastId; ++i)
        {
            ubo.materials[i] = materials[i];
        }
    }

    uint32_t GWMaterialHandler::createMaterial(float roughness, float metallic, glm::vec4 color, std::string name)
    {
        Material newMaterial{};
        newMaterial.data.z = lastId;
        newMaterial.data.y = roughness;
        newMaterial.data.x = metallic;
        newMaterial.color = color;

        materials[lastId] = newMaterial;

        MaterialData newMaterialData{};
        newMaterialData.name = name;
        newMaterialData.id = lastId;

        materialsData.at(lastId) = newMaterialData;

        lastId += 1;

        return newMaterial.data.z;
    }
}