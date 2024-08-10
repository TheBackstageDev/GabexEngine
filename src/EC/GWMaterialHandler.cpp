#include "GWMaterialHandler.hpp"

#include <iostream>

namespace GWIN
{
    GWMaterialHandler::GWMaterialHandler(GWinDevice& device) : device(device)
    {
        createMaterial(0.5f, .5f, {1.0f, 1.0f, 1.0f, 1.0f}); //Default material
    }

    void GWMaterialHandler::setMaterials(GlobalUbo& ubo)
    {
        for (uint32_t i = 0; i < lastId; ++i)
        {
            ubo.materials[i] = materials[i];
        }
    }

    uint32_t GWMaterialHandler::createMaterial(float roughness, float metallic, glm::vec4 color)
    {
        Material newMaterial{};
        newMaterial.data.z = lastId;
        newMaterial.data.y = roughness;
        newMaterial.data.x = metallic;
        newMaterial.color = color;

        materials[lastId] = newMaterial;

        lastId += 1;

        return newMaterial.data.z;
    }
}