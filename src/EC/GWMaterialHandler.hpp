#pragma once

#include "GWFrameInfo.hpp"
#include <array>

namespace GWIN
{
    struct MaterialData
    {
        std::string name = "Default Name";
        uint32_t id{0};
    };

    class GWMaterialHandler
    {
    public:
        GWMaterialHandler(GWinDevice& device);
        //~GWMaterialHandler();

        uint32_t createMaterial(float roughness, float metallic, glm::vec4 color, std::string name);
        void setMaterials(GlobalUbo& ubo);

        std::array<Material, MAX_MATERIALS> getMaterials() { return materials; }
        std::vector<MaterialData>& getMaterialData() { return materialsData; };

        void resetMaterials() { 
            for (auto &material : materials)
            {
                material = Material{}; 
            }
            materialsData.clear();
            lastId = 0; 
        };
    private:
        GWinDevice & device;
        std::array<Material, MAX_MATERIALS> materials;
        std::vector<MaterialData> materialsData{MAX_MATERIALS};

        uint32_t lastId{0};
    };
}