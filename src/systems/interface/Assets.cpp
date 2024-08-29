#include "Assets.hpp"

#include <iostream>
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

namespace GWIN
{
    AssetsWindow::AssetsWindow(std::unique_ptr<GWTextureHandler>& imageLoader, std::unique_ptr<GWMaterialHandler>& materialHandler) 
    : imageLoader(imageLoader), materialHandler(materialHandler)
    {
        createDefaultImages();
        createAsset(AssetInfo{"Amogus", ASSET_TYPE_TEXTURE});
    }

    AssetsWindow::~AssetsWindow()
    {
        for (auto &image : images)
        {
            ImGui_ImplVulkan_RemoveTexture(image.second);
        }
    }

    void AssetsWindow::actions(AssetType type)
    {
        switch(type)
        {
        default:
            std::cout << "Invalid asset type! \n";
        }
    }

    void AssetsWindow::draw()
    {
        if (ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_NoCollapse))
        {
            ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), true, ImGuiWindowFlags_NoResize);
            
            for (const auto& asset : assets)
            {
                ImGui::PushID(asset.id);
            

                ImGui::PopID();
            }

            ImGui::EndChild();

        } ImGui::End();
    }

    void AssetsWindow::createAsset(AssetInfo& assetInfo)
    {
        Asset newAsset{};
        newAsset.name = assetInfo.name;
        newAsset.type = assetInfo.type;
        newAsset.id = lastAssetID++;

        switch(assetInfo.type)
        {
        case ASSET_TYPE_MESH:
            break;
        case ASSET_TYPE_MATERIAL:
            break;
        case ASSET_TYPE_SCRIPT:
            break;
        case ASSET_TYPE_TEXTURE:
            break;
        default:
            std::cout << "Invalid asset type";
            break; 
        }

        assets.push_back(newAsset);
    }

    void AssetsWindow::removeAsset(std::string name)
    {

    }

    void AssetsWindow::createImage(const std::string &pathToFile)
    {
        std::regex fileNameRegex(R"(.*[\\/](.*)\..*$)");
        std::smatch matches;

        std::string fileName;
        if (std::regex_match(pathToFile, matches, fileNameRegex) && matches.size() > 1)
        {
            fileName = matches[1].str(); 
        } else {
            std::cout << "Incorrect file path!" << std::endl;
            return;
        }

        Texture NewImage = imageLoader->createTexture(std::string(pathToFile), false);

        if (NewImage.textureImage.imageView != nullptr && NewImage.textureSampler != nullptr)
        {
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
            descriptorSet = ImGui_ImplVulkan_AddTexture(NewImage.textureSampler, NewImage.textureImage.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            if (descriptorSet != VK_NULL_HANDLE)
            {
                images.emplace(fileName, descriptorSet);
            }
        }
    }

    void AssetsWindow::createDefaultImages()
    {
        std::string relativePath = "..\\src\\systems\\interface\\images";
        fs::path imageDirectory = fs::current_path() / relativePath;

        if (fs::exists(imageDirectory) && fs::is_directory(imageDirectory))
        {
            for (const auto &entry : fs::directory_iterator(imageDirectory))
            {
                if (entry.is_regular_file())
                {
                    std::cout << entry.path().string() << std::endl;
                    createImage(entry.path().string());
                }
            }
        }
    }

    void AssetsWindow::materialEditor()
    {

    }
}