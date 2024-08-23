#pragma once

#include "GWFrameInfo.hpp"
#include <array>

namespace GWIN
{
    class GWMaterialHandler
    {
    public:
        GWMaterialHandler(GWinDevice& device);
        //~GWMaterialHandler();

        uint32_t createMaterial(float roughness, float metallic, glm::vec4 color);
        void setMaterials(GlobalUbo& ubo);

        std::array<Material, MAX_MATERIALS> getMaterials() { return materials; }
        void resetMaterials() { 
            for (auto &material : materials)
            {
                material = Material{}; 
            }
            lastId = 0; 
        };
    private:
        GWinDevice & device;
        std::array<Material, MAX_MATERIALS> materials;

        uint32_t lastId{0};
    };
}