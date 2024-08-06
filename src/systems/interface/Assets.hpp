#pragma once

#include <vulkan/vulkan.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "GWTextureHandler.hpp"

namespace GWIN
{
    enum AssetType
    {
        ASSET_TYPE_MESH,
        ASSET_TYPE_SCRIPT,
        ASSET_TYPE_TEXTURE,
        ASSET_TYPE_MATERIAL
    };

    struct Asset
    {
        VkDescriptorSet image = VK_NULL_HANDLE;
        std::string name;
        AssetType type;
    };

    class AssetsWindow
    {
    public:
        AssetsWindow(GWTextureHandler& imageLoader);
        ~AssetsWindow();

        void draw();
    private:

        void actions(AssetType type);
        void materialEditor();

        //Future
        void meshVisualizator();

        void createDefaultImages();
        VkDescriptorSet createImage(const std::string& pathToFile);

        GWTextureHandler& imageLoader;

        std::vector<Asset> assets;
        std::unordered_map<std::string, VkDescriptorSet> images;
    };
}