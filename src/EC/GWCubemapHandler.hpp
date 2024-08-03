#pragma once

#include "GWImageLoader.hpp" 
#include <array>

namespace GWIN
{
    struct CubeMapInfo //Path to each of the cube map's faces
    {
        std::string posX;
        std::string posY; //Top
        std::string posZ;
        std::string negX;
        std::string negY; //Bottom
        std::string negZ; 

        std::array<std::string, 6> getFaces()
        {
            return {posX, posY, posZ, negX, negY, negZ};
        }
    };

    struct CubeMap
    {
        CubeMapInfo info;
        Image Cubeimage;
        uint32_t id;
    };

    class GWCubemapHandler
    {
    public:
        GWCubemapHandler(GWinDevice& device);
        ~GWCubemapHandler();
        
        GWCubemapHandler(const GWCubemapHandler &) = delete;
        GWCubemapHandler &operator=(const GWCubemapHandler &) = delete;

        CubeMap createCubeMap(CubeMapInfo &info);

    private:
        GWinDevice& device;

        void generateCubeMap(CubeMap &cubeMap);
        void createSampler(VkSampler& sampler);

        void transitionImageLayout(CubeMap &cubeMap, VkImageLayout newLayout);
        void createImageView(CubeMap& cubeMap);

        uint32_t lastCubemapId{0};

        std::vector<CubeMap> cubeMapsForDeletion;
    };
}