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
    }

    AssetsWindow::~AssetsWindow()
    {
        for (auto &image : images)
        {
            ImGui_ImplVulkan_RemoveTexture(image.second);
        }
    }

    void AssetsWindow::assetMenu()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));           
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.35f, 1.0f)); 
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));     

        if (ImGui::ImageButton((ImTextureID)images["plus_sign"], ImVec2(16, 16)))
        {
            ImGui::OpenPopup("AddComponent");
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        if (ImGui::BeginPopup("AddComponent"))
        {
            ImGui::Text("Add New Asset");

            ImGui::Separator();

            if (ImGui::MenuItem("Mesh"))
            {
                createAsset(AssetCreateInfo{"New Mesh", ASSET_TYPE_MESH});
            }
            if (ImGui::MenuItem("Material"))
            {
                createAsset(AssetCreateInfo{"New Material", ASSET_TYPE_MATERIAL});
            }
            if (ImGui::MenuItem("Script"))
            {
                createAsset(AssetCreateInfo{"New Script", ASSET_TYPE_SCRIPT});
            }
            if (ImGui::MenuItem("Texture"))
            {
                createAsset(AssetCreateInfo{"New Texture", ASSET_TYPE_TEXTURE});
            }
            if (ImGui::MenuItem("Sound"))
            {
                createAsset(AssetCreateInfo{"New Sound", ASSET_TYPE_SOUND});
            }

            ImGui::EndPopup();
        }
    }

    void AssetsWindow::drawAsset(Asset& asset)
    {
        bool isSelected = (selectedAsset == asset.id);
        std::string assetId = "Asset" + std::to_string(asset.id) + "##" + std::to_string(asset.id);

        if (isSelected && !selectedDisable)
        {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.3f, 0.5f, 1.0f, 0.5f)); 
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));    
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));  
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); // Default background color
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));    // Default text color
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));  // Default border color
        }

        ImGui::BeginChild(assetId.c_str(), ImVec2(117, 150), ImGuiChildFlags_Border, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiChildFlags_Border);

        if (asset.image != "" && images[asset.image] != VK_NULL_HANDLE)
        {
            ImGui::Image((ImTextureID)images[asset.image], ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
        }
        else
        {
            ImGui::Dummy(ImVec2(100, 100));
        }

        ImGui::NextColumn();

        ImVec2 textSize = ImGui::CalcTextSize(asset.name.c_str());
        ImVec2 availableRegion = ImGui::GetContentRegionAvail();
        float xOffset = (availableRegion.x - textSize.x) * 0.5f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xOffset);
        ImGui::Text("%s", asset.name.c_str());

        ImGui::NextColumn();

        std::string type;
        switch (asset.type)
        {
        case ASSET_TYPE_MESH:
            type = "MESH";
            break;
        case ASSET_TYPE_MATERIAL:
            type = "MATERIAL";
            break;
        case ASSET_TYPE_SCRIPT:
            type = "SCRIPT";
            break;
        case ASSET_TYPE_TEXTURE:
            type = "TEXTURE";
            break;
        case ASSET_TYPE_SOUND:
            type = "SOUND";
            break;
        default:
            type = "UNKNOWN";
        }

        std::transform(type.begin(), type.end(), type.begin(), ::toupper);
        textSize = ImGui::CalcTextSize(type.c_str());
        xOffset = (availableRegion.x - textSize.x) * 0.5f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xOffset);
        ImGui::Text("%s", type.c_str());

        ImGui::EndChild();

        ImGui::PopStyleColor(3);
    }
    
    void AssetsWindow::draw()
    {
        ImGui::SetNextWindowDockID(ImGui::GetID("##Dockspace"), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_NoScrollbar))
        {
            assetMenu();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
            ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_HorizontalScrollbar);

            if (hasNewAssetBeenSelected)
                hasNewAssetBeenSelected = false;

            for (int i = 0; i < assets.size(); ++i)
            {
                drawAsset(assets[i]);
                if (ImGui::IsItemClicked())
                {
                    hasNewAssetBeenSelected = true;
                    selectedAsset = i;
                }
                ImGui::SameLine();
            }

            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            ImGui::End();
        }
    }

    void AssetsWindow::createAsset(AssetCreateInfo &assetInfo)
    {
        Asset newAsset{};
        newAsset.name = assetInfo.name;
        newAsset.type = assetInfo.type;
        newAsset.id = lastAssetID++;

        uint32_t index;

        switch(assetInfo.type)
        {
        case ASSET_TYPE_MESH:
            //Temporary
            break;
        case ASSET_TYPE_MATERIAL:
            index = materialHandler->createMaterial(0.5, 0.5, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), std::string("New Material"));
            newAsset.info.index = index;
            break;
        case ASSET_TYPE_SCRIPT:
            newAsset.image = "script";
            break;
        case ASSET_TYPE_TEXTURE:
            break;
        case ASSET_TYPE_SOUND:
            newAsset.image = "sound";
            break;
        default:
            std::cout << "Invalid asset type";
            --lastAssetID;
            return;
        }

        assets.push_back(newAsset);
    }

    void AssetsWindow::removeAsset(uint32_t id)
    {
        auto& asset = assets.at(id);
        switch (asset.type)
        {
        case ASSET_TYPE_MESH:
            break;
        case ASSET_TYPE_MATERIAL:
            break;
        case ASSET_TYPE_SCRIPT:
            break;
        case ASSET_TYPE_TEXTURE:
            break;
        case ASSET_TYPE_SOUND:
            break;
        default:
            std::cerr << "Invalid asset type!";
            return;
        }
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
}