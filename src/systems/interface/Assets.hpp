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
        ASSET_TYPE_MATERIAL,
        ASSET_TYPE_SOUND
    };

    struct AssetInfo
    {
        std::string pathToFile{"None"};
        int32_t index{0}; // what X object it points to, such as material.
    };

    struct Asset
    {
        std::string image{""}; // 0 is no image;
        std::string name = "Default Name";
        uint32_t id{0};
        AssetInfo info{};
        AssetType type{ASSET_TYPE_MESH};
    };

    struct AssetCreateInfo 
    {
        std::string name;
        AssetType type;
    };

    class AssetsWindow
    {
    public:
        AssetsWindow(std::unique_ptr<GWTextureHandler>& imageLoader, std::unique_ptr<GWMaterialHandler>& materialHandler);
        ~AssetsWindow();

        void draw(FrameInfo &frameInfo);

        std::unordered_map<std::string, VkDescriptorSet>& getImages() { return images; }
        int32_t getSelectedAsset() { return selectedAsset; }

        std::vector<Asset>& getAssets() { return assets; }

        bool isNewAssetSelected() { return hasNewAssetBeenSelected; }
        void setDisable(bool isDisabled) { selectedDisable = isDisabled; }

        void deselect() { selectedAsset = -1; }

        std::unique_ptr<GWTextureHandler>& getTextureHandler() { return imageLoader; }
    private:

        //Future
        void scriptVisualizator();
        void meshVisualizator();

        void createDefaultImages();
        void createImage(const std::string& pathToFile);

        void createAsset(AssetCreateInfo& assetInfo);
        void removeAsset(uint32_t id);

        void drawAsset(Asset& asset, FrameInfo& frameInfo);
        void assetMenu();

        std::unique_ptr<GWTextureHandler>& imageLoader;
        std::unique_ptr<GWMaterialHandler>& materialHandler;

        std::vector<Asset> assets;
        std::unordered_map<std::string, VkDescriptorSet> images;

        int32_t selectedAsset{-1}; //-1 for none
        uint32_t lastAssetID{0};

        bool hasNewAssetBeenSelected{false};
        bool selectedDisable{false};
    };
}