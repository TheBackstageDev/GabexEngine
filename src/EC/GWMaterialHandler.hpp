#pragma once

#include "GWFrameInfo.hpp"
#include <vector>

namespace GWIN
{
    class GWMaterialHandler
    {
    public:
        GWMaterialHandler(GWinDevice& device);
        //~GWMaterialHandler();

        uint32_t createMaterial(float roughness, float metallic, glm::vec4 color);
        void setMaterials(GlobalUbo& ubo);
    private:
        GWinDevice& device;
        std::vector<Material> materials{MAX_MATERIALS};

        uint32_t lastId{0};
    };
}