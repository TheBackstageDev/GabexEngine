#pragma once

#include <vulkan/vulkan.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "GWTextureHandler.hpp"
#include "GWMaterialHandler.hpp"

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
        std::string name = "Default Name";
        uint32_t id{0};
        AssetType type{ASSET_TYPE_MESH};
    };

    struct AssetInfo 
    {
        std::string name;
        AssetType type;
    };

    class AssetsWindow
    {
    public:
        AssetsWindow(std::unique_ptr<GWTextureHandler>& imageLoader, std::unique_ptr<GWMaterialHandler>& materialHandler);
        ~AssetsWindow();

        void draw();
    private:
        void actions(AssetType type);
        void materialEditor();

        //Future
        void meshVisualizator();

        void createDefaultImages();
        void createImage(const std::string& pathToFile);

        void createAsset(AssetInfo& assetInfo);
        void removeAsset(std::string name);

        std::unique_ptr<GWTextureHandler>& imageLoader;
        std::unique_ptr<GWMaterialHandler>& materialHandler;

        std::vector<Asset> assets;
        std::unordered_map<std::string, VkDescriptorSet> images;

        uint32_t lastAssetID{0};
    };
}