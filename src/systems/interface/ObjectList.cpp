#include "ObjectList.hpp"

namespace GWIN
{
    void GWObjectList::inputPosition(GWGameObject &selectedObject)
    {
        ImGui::Text("Position:");

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Button("X");
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 0); 
        ImGui::SetNextItemWidth(80);
        if (ImGui::DragFloat("##PosX", &positionBuffer.x, 0.1f, -FLT_MAX, FLT_MAX, "%.1f"))
        {
            selectedObject.transform.translation.x = positionBuffer.x;
        }

        ImGui::SameLine(0, 0); 
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
        ImGui::Button("Y");
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 0);
        ImGui::SetNextItemWidth(80);
        if (ImGui::DragFloat("##PosY", &positionBuffer.y, 0.1f, -FLT_MAX, FLT_MAX, "%.1f"))
        {
            selectedObject.transform.translation.y = positionBuffer.y;
        }

        ImGui::SameLine(0, 0); 
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::Button("Z");
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 0);
        ImGui::SetNextItemWidth(80);
        if (ImGui::DragFloat("##PosZ", &positionBuffer.z, 0.1f, -FLT_MAX, FLT_MAX, "%.1f"))
        {
            selectedObject.transform.translation.z = positionBuffer.z;
        }
    }

    void GWObjectList::inputRotation(GWGameObject &selectedObject)
    {
        ImGui::Text("Rotation:");

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Button("X");
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 0); 
        ImGui::SetNextItemWidth(80);
        if (ImGui::DragFloat("##RotX", &rotationBuffer.x, .2f, -360.0f, 360.0f, "%.1f°"))
        {
            selectedObject.transform.rotation.x = glm::radians(rotationBuffer.x);
        }

        ImGui::SameLine(0, 0); 
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
        ImGui::Button("Y");
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 0); 
        ImGui::SetNextItemWidth(80);
        if (ImGui::DragFloat("##RotY", &rotationBuffer.y, .2f, -360.0f, 360.0f, "%.1f°"))
        {
            selectedObject.transform.rotation.y = glm::radians(rotationBuffer.y);
        }

        ImGui::SameLine(0, 0); 
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::Button("Z");
        ImGui::PopStyleColor();
        ImGui::SameLine(0, 0); 
        ImGui::SetNextItemWidth(80);
        if (ImGui::DragFloat("##RotZ", &rotationBuffer.z, .2f, -360.0f, 360.0f, "%.1f°"))
        {
            selectedObject.transform.rotation.z = glm::radians(rotationBuffer.z);
        }
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
            //Temporary
            static const char *materials[] = {"Material1", "Material2", "Material3"};
            static int selectedMaterial = 0;

            ImGui::Text("Material");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150);
            if (ImGui::BeginCombo("##MaterialCombo", materials[selectedMaterial]))
            {
                for (int i = 0; i < IM_ARRAYSIZE(materials); i++)
                {
                    bool isSelected = (selectedMaterial == i);
                    if (ImGui::Selectable(materials[i], isSelected))
                    {
                        selectedMaterial = i;
                    }
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }
    }

    void GWObjectList::addComponent(FrameInfo& frameInfo)
    {

    }

    void GWObjectList::Draw(FrameInfo &frameInfo)
    {
        auto io = ImGui::GetIO();

        ImGui::SetNextWindowDockID(ImGui::GetID("Instance"), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(0, 20));
        if (ImGui::Begin("Scene Hierarchy##", nullptr))
        {
            ImGui::BeginChild("ScrollingRegion##1", ImVec2(0, 0), true);

            for (auto &kv : frameInfo.gameObjects)
            {
                std::string objId = kv.first + " " + kv.second.getName() + "##" + (char)kv.first;
                if (kv.first == selectedItem)
                {
                    positionBuffer = kv.second.transform.translation;
                    rotationBuffer = glm::degrees(kv.second.transform.rotation);
                    scaleBuffer = kv.second.transform.scale;
                }

                if (ImGui::Selectable(objId.c_str(), selectedItem == kv.second.getId()))
                {
                    selectedItem = kv.second.getId(); //Sets to its Index
                    strncpy_s(nameBuffer, kv.second.getName().c_str(), sizeof(nameBuffer) - 1);
                    nameBuffer[sizeof(nameBuffer) - 1] = '\0'; 
                    isEditingName = false;
                }
            }

            ImGui::EndChild();
            ImGui::End();
        }

        ImGui::SetNextWindowDockID(ImGui::GetID("Instance"), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoCollapse))
        {
            ImGui::BeginChild("ScrollingRegion##2", ImVec2(0, 0), true, ImGuiWindowFlags_NoResize);
            if (selectedItem != -1)
            {
                GWGameObject &selectedObject = frameInfo.gameObjects.at(selectedItem);
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
            } else {
                ImGui::Text("Select a Object");
            }

            //addComponent(frameInfo);

            ImGui::EndChild();
        }
        ImGui::End();
    }
}