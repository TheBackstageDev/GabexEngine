#include "ObjectList.hpp"

#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include "./Console.hpp"
#include "GWTextureHandler.hpp"

namespace GWIN
{
    bool InputXYZ(const char* id, const char *label, float &value, const ImVec4 &buttonColor)
    {
        ImGui::PushID(id);
        ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
        ImGui::Button(label);
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 0);
        ImGui::SetNextItemWidth(80);
        bool valueChanged = ImGui::DragFloat("##hidden", &value, 0.1f, -FLT_MAX, FLT_MAX, "%.1f");
        ImGui::PopID();
        return valueChanged;
    }

    void GWObjectList::inputPosition(GWGameObject &selectedObject)
    {
        ImGui::Text("Position:");
        bool positionChanged = false;

        positionChanged |= InputXYZ("PosX", "X", positionBuffer.x, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::SameLine(0, 0);
        positionChanged |= InputXYZ("PosY", "Y", positionBuffer.y, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
        ImGui::SameLine(0, 0);
        positionChanged |= InputXYZ("PosZ", "Z", positionBuffer.z, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

        if (positionChanged)
        {
            selectedObject.transform.translation = positionBuffer;
        }
    }

    void GWObjectList::inputRotation(GWGameObject &selectedObject)
    {
        ImGui::Text("Rotation:");
        rotationChanged = false;

        rotationChanged |= InputXYZ("RotX", "X", rotationBuffer.x, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::SameLine(0, 0);
        rotationChanged |= InputXYZ("RotY", "Y", rotationBuffer.y, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
        ImGui::SameLine(0, 0);
        rotationChanged |= InputXYZ("RotZ", "Z", rotationBuffer.z, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
    }

    void CenteredText(const char *text)
    {
        float textWidth = ImGui::CalcTextSize(text).x;
        float availableWidth = ImGui::GetContentRegionAvail().x;
        float padding = (availableWidth - textWidth) / 2.0f;

        if (padding > 0.0f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);

        ImGui::Text("%s", text);
    }

    void GWObjectList::inputModel(GWGameObject &selectedObject)
    {
        if (selectedObject.model == -1)
            return;

        if (ImGui::CollapsingHeader("Model", nullptr))
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.6f));
            if (ImGui::Button("Remove Model"))
            {
                selectedObject.model = -1; 
                GWConsole::addLog("Model removed from the selected object.");
            }
            ImGui::PopStyleColor();

            std::string modelName = "Placeholder Code";
            ImGui::Text("Current Model: %s", modelName.c_str());
        }
    }

    void GWObjectList::inputTexture(std::shared_ptr<GWModel> &selectedObject)
    {
        if (selectedObject->Textures[0] == 0)
            return;

        if (ImGui::CollapsingHeader("Texture", nullptr))
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.6f));
            if (ImGui::Button("Remove Texture"))
            {
                selectedObject->Textures[0] = 0;
                GWConsole::addLog("Texture removed from the selected object.");
            }
            ImGui::PopStyleColor();

            std::string textureName = "Placeholder Code";
            ImGui::Text("Current Model: %s", textureName.c_str());
        }
    }

    void GWObjectList::inputMaterial(std::shared_ptr<GWModel> &selectedObject)
    {
        if (selectedObject->Material == 0)
            return;

        if (ImGui::CollapsingHeader("Material", nullptr))
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.6f));
            if (ImGui::Button("Remove Material"))
            {
                selectedObject->Material = 0; 
                GWConsole::addLog("Material removed from the selected object.");
            }
            ImGui::PopStyleColor();

            std::string materialName = "Placeholder Code";
            ImGui::Text("Current Model: %s", materialName.c_str());
        }
    }

    float cutOffAngleBuffer = 0.0f;
    void GWObjectList::inputLight(GWGameObject &selectedObject)
    {
        if (ImGui::CollapsingHeader("Light", nullptr))
        {
            if (selectedObject.light->cutOffAngle != 0.0f)
            {
                cutOffAngleBuffer = glm::degrees(selectedObject.light->cutOffAngle);
                
                ImGui::Text("Type: Spotlight");
                ImGui::DragFloat("CutOff Angle: ", &cutOffAngleBuffer, .1f, 0.1f, 90.f, "%.1f");

                selectedObject.light->cutOffAngle = glm::radians(cutOffAngleBuffer);
            }
            else
            {
                ImGui::Text("Type: Pointlight");
            }

            ImGui::DragFloat("Intensity: ", &selectedObject.light->lightIntensity, .1f, 0.f, FLT_MAX, "%.1f");
        }
    }

    void GWObjectList::inspectorGuis(GWGameObject &selectedObject, FrameInfo& frameInfo)
    {
        if (ImGui::CollapsingHeader("Transform", nullptr))
        {
            inputPosition(selectedObject);
            inputRotation(selectedObject);

            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Scale:");

            bool scaleChanged = InputXYZ("ScaleX", "X", scaleBuffer.x, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::SameLine(0, 0);
            scaleChanged |= InputXYZ("ScaleY", "Y", scaleBuffer.y, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
            ImGui::SameLine(0, 0);
            scaleChanged |= InputXYZ("ScaleZ", "Z", scaleBuffer.z, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

            if (scaleChanged)
            {
                selectedObject.transform.scale = scaleBuffer;
            }
        }

        if (selectedObject.light == nullptr)
        {
            if (selectedObject.model != -1)
            {
                auto &model = frameInfo.currentInfo.meshes.at(selectedObject.model);
                inputModel(selectedObject);
                inputTexture(model);
                inputMaterial(model);
            }
        }
        else
        {
            inputLight(selectedObject);
        }
    }

    // Asset-Related
    char buffer[128];
    void GWObjectList::materialEditor(Asset &selectedAsset)
    {
        ImGui::InputText("Material Name", (char *)selectedAsset.name.c_str(), selectedAsset.name.size() + 1, ImGuiInputTextFlags_EnterReturnsTrue);

        auto &material = materialHandler->getMaterial(selectedAsset.info.index);
        auto &materialData = materialHandler->getMaterialDataAt(selectedAsset.info.index);

        materialData.name = selectedAsset.name;

        ImGui::ColorEdit4("Diffuse Color", glm::value_ptr<float>(material.color));
        ImGui::Spacing();
        ImGui::SliderFloat("Metalicity", &material.data.x, 0.0f, 1.0f, "%.1f");
        ImGui::Spacing();
        ImGui::SliderFloat("Roughness", &material.data.y, 0.0f, 1.0f, "%.1f");

        ImGui::Separator();
        CenteredText("Material Visualization");
        ImGui::Image((ImTextureID)assets->getImages().at("scale"), ImGui::GetContentRegionAvail());
    }

    void GWObjectList::meshEditor(Asset &selectedAsset, FrameInfo &frameInfo)
    {
        strncpy_s(buffer, selectedAsset.name.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';

        if (ImGui::InputText("Mesh Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            selectedAsset.name = buffer;
        }

        ImGui::Text("Source: ");
        ImGui::SameLine();
        std::string path = selectedAsset.info.pathToFile;
        if (path.length() > 20)
        {
            path = "..." + path.substr(path.length() - 20);
        }
        ImGui::Text(path.c_str());

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));                    // Transparent background
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.5f)); // Semi-transparent when hovered
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));              // Transparent when clicked

        if (ImGui::ImageButton((ImTextureID)assets->getImages().at("file"), ImVec2(20, 20)))
        {
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".obj,.fbx,.gltf,.glb,.");
        }

        ImGui::PopStyleColor(3);

        if (selectedAsset.info.pathToFile != "None")
        {
            auto& model = frameInfo.currentInfo.meshes.at(selectedAsset.info.index);

            std::string numVertices = std::to_string(model->numVertices());
            std::string numTris = std::to_string(model->numVertices() / 3);

            ImGui::Text("Num Vertices: %s", numVertices.c_str());
            ImGui::Text("Num Triangles: %s", numTris.c_str());
        } else {
            ImGui::Text("Num Vertices: None");
            ImGui::Text("Num Triangles: None");
        }

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string fullPath = ImGuiFileDialog::Instance()->GetFilePathName();
                selectedAsset.info.pathToFile = fullPath;

                if (fullPath.length() > 20)
                {
                    path = "..." + fullPath.substr(fullPath.length() - 20);
                }
                else
                {
                    path = fullPath;
                }

                if (selectedAsset.info.index != -1)
                {
                    removeMeshCallback(selectedAsset.info.index);
                    createMeshCallback(fullPath, selectedAsset.info.index);
                    GWConsole::addLog("Mesh ID: " + std::to_string(selectedAsset.info.index));
                } else {
                    selectedAsset.info.index = createMeshCallback(fullPath, std::nullopt);
                    GWConsole::addLog("New Mesh ID: " + std::to_string(selectedAsset.info.index));
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        CenteredText("Mesh Visualization");
        ImGui::Image((ImTextureID)assets->getImages().at("scale"), ImGui::GetContentRegionAvail());
    }

    void GWObjectList::textureEditor(Asset &selectedAsset, FrameInfo &frameInfo)
    {
        strncpy_s(buffer, selectedAsset.name.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';

        if (ImGui::InputText("Texture Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            selectedAsset.name = buffer;
        }

        ImGui::Text("Source: ");
        ImGui::SameLine();
        std::string path = selectedAsset.info.pathToFile;
        if (path.length() > 20)
        {
            path = "..." + path.substr(path.length() - 20);
        }
        ImGui::Text(path.c_str());

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));                    // Transparent background
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.5f)); // Semi-transparent when hovered
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));              // Transparent when clicked

        if (ImGui::ImageButton((ImTextureID)assets->getImages().at("file"), ImVec2(20, 20)))
        {
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png,.jpg,.jpeg,.tga,.bmp,.");
        }

        ImGui::PopStyleColor(3);

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string fullPath = ImGuiFileDialog::Instance()->GetFilePathName();
                selectedAsset.info.pathToFile = fullPath;
                if (fullPath.length() > 20)
                {
                    path = "..." + fullPath.substr(fullPath.length() - 20);
                }
                else
                {
                    path = fullPath;
                }

                Texture newTexture;

                if (selectedAsset.info.index == 0)
                {
                    newTexture = assets->getTextureHandler()->createTexture(fullPath, true);

                    createTextureCallback(newTexture, false);

                    selectedAsset.info.index = newTexture.id;
                } else {
                    assets->getTextureHandler()->destroyTexture(selectedAsset.info.index);
                    newTexture = assets->getTextureHandler()->createTexture(fullPath, true);

                    newTexture.id = selectedAsset.info.index;

                    assets->getTextureHandler()->decreaseLastTextureID();

                    createTextureCallback(newTexture, true);
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        CenteredText("Texture Visualization");
        //ImGui::Image((ImTextureID)frameInfo.currentInfo.textures[selectedAsset.info.index], ImGui::GetContentRegionAvail());
    }

    void GWObjectList::assetList(FrameInfo &frameInfo)
    {
        assets->setDisable(false);
        selectedItem = assets->getSelectedAsset();
        if (selectedItem != -1)
        {
            auto& assetsList = assets->getAssets();

            Asset& currentAsset = assetsList[selectedItem];
            uint32_t type = assetsList[selectedItem].type;
            switch (type)
            {
            case ASSET_TYPE_MATERIAL:
                materialEditor(currentAsset);
                break;
            case ASSET_TYPE_MESH:
                meshEditor(currentAsset, frameInfo);
                break;
            case ASSET_TYPE_SCRIPT:
                ImGui::Text("Is a Script");
                break;
            case ASSET_TYPE_TEXTURE:
                textureEditor(currentAsset, frameInfo);
                break;
            case ASSET_TYPE_SOUND:
                ImGui::Text("Is a Sound");
                break;
            default:
                std::cout << "Invalid asset type! \n";
            }
        }
        else
        {
            ImGui::Text("Select an Asset");
        }
    }

/*  void GWObjectList::createComponentAsset(const Asset& selectedAsset, FrameInfo& frameInfo)
    {
        ImGui::Text(selectedAsset.name.c_str());
    } */

    void GWObjectList::addComponent(FrameInfo& frameInfo)
    {
        //Component Button
        ImVec2 buttonSize(ImGui::GetContentRegionAvail().x * 0.75f, 20);
        float windowWidth = ImGui::GetContentRegionAvail().x;
        float windowHeight = ImGui::GetWindowHeight();
        float buttonPosX = (windowWidth - buttonSize.x) / 2.0f;                             
        float buttonPosY = windowHeight - buttonSize.y - ImGui::GetStyle().WindowPadding.y; 

        ImGui::SetCursorPosX(buttonPosX);
        ImGui::SetCursorPosY(buttonPosY);
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 1.0f, .25f, .6f));
        if (ImGui::Button("Add Component", ImVec2(windowWidth * .75, 20)))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }
        ImGui::PopStyleColor();

        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            auto& currentAssets = assets->getAssets();

            std::unordered_map<uint32_t, std::vector<Asset>> assetsMap = {
                {ASSET_TYPE_MESH, {}},
                {ASSET_TYPE_TEXTURE, {}},
                {ASSET_TYPE_MATERIAL, {}}};

            for (const auto &asset : currentAssets)
            {
                assetsMap[asset.type].push_back(asset);
            }

            std::vector<Asset> &meshAssets = assetsMap[ASSET_TYPE_MESH];
            std::vector<Asset> &textureAssets = assetsMap[ASSET_TYPE_TEXTURE];
            std::vector<Asset> &materialAssets = assetsMap[ASSET_TYPE_MATERIAL];

            auto &currentObject = frameInfo.currentInfo.gameObjects.at(selectedItem);

            if (ImGui::BeginCombo("##MeshCombo", "Select Mesh"))
            {
                if (meshAssets.empty())
                {
                    ImGui::Text("No Meshes Created!");
                }
                else
                {
                    for (const auto &asset : meshAssets)
                    {
                        bool isSelected = (asset.info.index == currentObject.model);
                        if (ImGui::Selectable(asset.name.c_str(), isSelected))
                        {
                            currentObject.model = asset.info.index;
                            ImGui::CloseCurrentPopup(); 
                        }
                    }
                }
                ImGui::EndCombo();
            }

            if (ImGui::BeginCombo("##TextureCombo", "Select Texture"))
            {
                if (textureAssets.empty() || currentObject.model == -1)
                {
                    ImGui::Text("No Textures Created!");
                }
                else
                {
                    auto &currentModel = frameInfo.currentInfo.meshes.at(currentObject.model);
                    for (const auto &asset : textureAssets)
                    {
                        bool isSelected = (asset.info.index == currentModel->Textures[0]);
                        if (ImGui::Selectable(asset.name.c_str(), isSelected))
                        {
                            currentModel->Textures[0] = asset.info.index;
                            ImGui::CloseCurrentPopup();
                        }
                    }
                }
                ImGui::EndCombo();
            }

            if (ImGui::BeginCombo("##MaterialCombo", "Select Material"))
            {
                if (materialAssets.empty() || currentObject.model == -1)
                {
                    ImGui::Text("No Materials Created!");
                }
                else
                {
                    auto &currentModel = frameInfo.currentInfo.meshes.at(currentObject.model);
                    for (const auto &asset : materialAssets)
                    {
                        bool isSelected = (asset.info.index == currentModel->Material);
                        if (ImGui::Selectable(asset.name.c_str(), isSelected))
                        {
                            currentModel->Material = asset.info.index;
                            ImGui::CloseCurrentPopup();  
                        }
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::EndPopup();
        }
    }

    void GWObjectList::Draw(FrameInfo &frameInfo)
    {
        auto io = ImGui::GetIO();

        ImGui::SetNextWindowDockID(ImGui::GetID("##Dockspace"), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Scene Hierarchy##", nullptr))
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.1f, 1.0f, .0f, .5f));
            if (ImGui::ImageButton((ImTextureID)assets->getImages().at("plus_sign"), ImVec2(16, 16)))
            {
                ImGui::OpenPopup("##addObjectPopup");
            }
            ImGui::PopStyleColor();

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, .1f, 0.f, .5f));
            if (ImGui::ImageButton((ImTextureID)assets->getImages().at("minus_sign"), ImVec2(16, 16)))
            {
                if (selectedItem != 0 && selectedItem != 1) // Skybox and directional Light
                {
                    removeObjectCallback(selectedItem);
                    assets->deselect();

                    selectedItem = -1;
                }
            }
            ImGui::PopStyleColor();

            if (ImGui::BeginPopup("##addObjectPopup"))
            {
                if (ImGui::BeginCombo("##Object", "Objects"))
                {
                    if (ImGui::Button("Empty"))
                    {
                        createObjectCallback(GameObjectType::BasicObject);
                        ImGui::CloseCurrentPopup();
                    }

                    if (ImGui::Button("Plane"))
                    {
                        ImGui::CloseCurrentPopup();
                    }

                    if (ImGui::Button("Cube"))
                    {
                        ImGui::CloseCurrentPopup();
                    }

                    if (ImGui::Button("Sphere"))
                    {
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndCombo();
                }

                if (ImGui::BeginCombo("##Light", "Light"))
                {
                    if (ImGui::Button("PointLight"))
                    {
                        createObjectCallback(GameObjectType::PointLight);
                        ImGui::CloseCurrentPopup();
                    }

                    if (ImGui::Button("SpotLight"))
                    {
                        createObjectCallback(GameObjectType::SpotLight);
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndCombo();
                }
                
                ImGui::EndPopup();
            }

            ImGui::BeginChild("##ScrollingRegion1", ImVec2(0, 0), ImGuiChildFlags_None);

            for (auto &kv : frameInfo.currentInfo.gameObjects)
            {
                std::string objId = kv.second.getName() + "##" + (char)kv.first;
                if (kv.first == selectedItem)
                {
                    positionBuffer = kv.second.transform.translation;
                    rotationBuffer = kv.second.transform.getRotation();
                    scaleBuffer = kv.second.transform.scale;
                }

                if (ImGui::Selectable(objId.c_str(), (selectedItem == kv.second.getId()) && !AssetSelected))
                {
                    selectedItem = kv.second.getId(); 
                    strncpy_s(nameBuffer, kv.second.getName().c_str(), sizeof(nameBuffer) - 1);
                    nameBuffer[sizeof(nameBuffer) - 1] = '\0';
                    isEditingName = false;
                    AssetSelected = false;
                    assets->setDisable(true);
                }
            }
        
            ImGui::EndChild();
            ImGui::End();
        }

        ImGui::SetNextWindowDockID(ImGui::GetID("##Dockspace"), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Inspector##", nullptr, ImGuiWindowFlags_NoCollapse))
        {
            ImGui::BeginChild("##ScrollingRegion2", ImVec2(0, 0), ImGuiChildFlags_None);

            if (selectedItem != -1 || assets->getSelectedAsset() != -1)
            {
                // Only sets isAssetSelected to true if the last selected
                // Asset is not still Active
                if (assets->isNewAssetSelected() && ((assets->getSelectedAsset() != selectedItem) && !AssetSelected)) 
                    AssetSelected = true;                                                                             

                if (!AssetSelected) 
                {
                    GWGameObject &selectedObject = frameInfo.currentInfo.gameObjects.at(selectedItem);
                    glm::vec3 position = selectedObject.transform.translation;

                    if (!isEditingName)
                    {
                        std::string name = selectedObject.getName();
                        auto nameDisplay = "Name: " + name;
                        if (ImGui::Selectable(nameDisplay.c_str(), isEditingName))
                        {
                            isEditingName = true;
                        }
                    }
                    else
                    {
                        if (ImGui::InputText("##Name", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                        {
                            selectedObject.setName(std::string(nameBuffer));
                            isEditingName = false;
                        }
                    }

                    inspectorGuis(selectedObject, frameInfo);

                    if (rotationChanged)
                    {
                        selectedObject.transform.rotate(rotationBuffer);
                    }

                    addComponent(frameInfo);
                }
                else 
                {
                    assetList(frameInfo);
                }
            }
            else
            {
                ImGui::Text("Select an Object or Asset");
            }

            ImGui::EndChild();

            ImGui::End();
        }
    }
}
