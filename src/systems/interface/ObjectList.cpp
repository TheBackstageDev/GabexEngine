#include "ObjectList.hpp"

#include <iostream>
#include <glm/gtc/type_ptr.hpp>

namespace GWIN
{
    void InputXYZ(const char *label, float &value, const ImVec4 &buttonColor)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
        ImGui::Button(label);
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 0);
        ImGui::SetNextItemWidth(80);
        ImGui::DragFloat(std::string("##").append(label).c_str(), &value, 0.1f, -FLT_MAX, FLT_MAX, "%.1f");
    }

    void GWObjectList::inputPosition(GWGameObject &selectedObject)
    {
        ImGui::Text("Position:");
        InputXYZ("X", positionBuffer.x, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        InputXYZ("Y", positionBuffer.y, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
        InputXYZ("Z", positionBuffer.z, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

        // Update selected object's position
        selectedObject.transform.translation = positionBuffer;
    }

    void GWObjectList::inputRotation(GWGameObject &selectedObject)
    {
        ImGui::Text("Rotation:");
        rotationChanged = false;

        rotationChanged |= ImGui::DragFloat("##RotX", &rotationBuffer.x, 0.2f, -360.0f, 360.0f, "%.1f°");
        rotationChanged |= ImGui::DragFloat("##RotY", &rotationBuffer.y, 0.2f, -360.0f, 360.0f, "%.1f°");
        rotationChanged |= ImGui::DragFloat("##RotZ", &rotationBuffer.z, 0.2f, -360.0f, 360.0f, "%.1f°");
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

    void GWObjectList::inspectorGuis(GWGameObject &selectedObject)
    {
        if (ImGui::CollapsingHeader("Transform", nullptr))
        {
            inputPosition(selectedObject);
            inputRotation(selectedObject);

            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Scale: ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150);
            if (ImGui::DragFloat("##Scale", &scaleBuffer, 0.05f, 0.1f, 10.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue))
            {
                selectedObject.transform.scale = scaleBuffer;
            }
        }

        if (ImGui::CollapsingHeader("Material", nullptr))
        {
            auto &materials = materialHandler->getMaterialData();
            selectedMaterial = selectedObject.Material;

            if (!materials.empty())
            {
                if (selectedMaterial >= materials.size())
                {
                    selectedMaterial = 0;
                }

                ImGui::Text("Material");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(150);
                if (ImGui::BeginCombo("##MaterialCombo", materials[selectedMaterial].name.c_str()))
                {
                    for (int i = 0; i < materials.size(); i++)
                    {
                        if (i != 0 && materials[i].id == 0)
                            continue;

                        bool isSelected = (selectedMaterial == i);
                        if (ImGui::Selectable(materials[i].name.c_str(), isSelected))
                        {
                            selectedMaterial = i;
                            selectedObject.Material = materials[i].id;
                        }
                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }
            else
            {
                ImGui::Text("No materials available.");
            }
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
            auto& object = frameInfo.currentInfo.gameObjects.at(selectedAsset.info.index);
            auto& model = frameInfo.currentInfo.meshes.at(object.model);
            char *vertexCount = (char *)model->numVertices();
            char* triangleCount = (char*)(model->numVertices() / 3);

            ImGui::Text("Num Vertices: " + *vertexCount);
            ImGui::Text("Num Triangles: " + *triangleCount);
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
            }
            ImGuiFileDialog::Instance()->Close();
        }

        CenteredText("Texture Visualization");
        ImGui::Image((ImTextureID)frameInfo.currentInfo.textures[selectedAsset.info.index], ImGui::GetContentRegionAvail());
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

    void GWObjectList::addComponent(FrameInfo& frameInfo)
    {

    }

    void GWObjectList::Draw(FrameInfo &frameInfo)
    {
        auto io = ImGui::GetIO();

        ImGui::SetNextWindowDockID(ImGui::GetID("##Dockspace"), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Scene Hierarchy##", nullptr))
        {
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

                    inspectorGuis(selectedObject);

                    if (rotationChanged)
                    {
                        // Calculate the difference in rotation to get the angle
                        glm::vec3 rotationDelta = rotationBuffer - selectedObject.transform.getRotation();

                        float angle = glm::length(rotationDelta);
                        if (angle > glm::epsilon<float>())
                        {
                            selectedObject.transform.rotate(glm::normalize(rotationDelta), angle);
                        }
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
