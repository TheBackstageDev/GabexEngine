#include "ObjectList.hpp"

namespace GWIN
{
    void GWObjectList::inputPosition(glm::vec3 &positionBuffer, GWGameObject &selectedObject)
    {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Position:");
        if (ImGui::InputFloat("##PosX", &positionBuffer.x, 0.1f, 1.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
        {
            selectedObject.transform.translation.x = positionBuffer.x;
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "X");

        if (ImGui::InputFloat("##PosY", &positionBuffer.y, 0.1f, 1.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
        {
            selectedObject.transform.translation.y = positionBuffer.y;
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "Y");

        if (ImGui::InputFloat("##PosZ", &positionBuffer.z, 0.1f, 1.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
        {
            selectedObject.transform.translation.z = positionBuffer.z;
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Z");
    }

    void GWObjectList::inputRotation(glm::vec3& rotationBuffer, GWGameObject& selectedObject)
    {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Rotation:");

        if (ImGui::SliderFloat("##RotX", &rotationBuffer.x, 0.0f, 360.0f, "%.2f"))
        {
            selectedObject.transform.rotation.x = glm::radians(rotationBuffer.x);
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "X");

        if (ImGui::SliderFloat("##RotY", &rotationBuffer.y, 0.0f, 360.0f, "%.2f"))
        {
            selectedObject.transform.rotation.y = glm::radians(rotationBuffer.y);
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "Y");

        if (ImGui::SliderFloat("##RotZ", &rotationBuffer.z, 0.0f, 360.0f, "%.2f"))
        {
            selectedObject.transform.rotation.z = glm::radians(rotationBuffer.z);
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Z");
    }

    void GWObjectList::Draw(FrameInfo &frameInfo)
    {
        auto io = ImGui::GetIO();
        static bool isEditingName = false;
        static char nameBuffer[1000] = "";
        static glm::vec3 positionBuffer = {0.0f, 0.0f, 0.0f};
        static glm::vec3 rotationBuffer = {0.0f, 0.0f, 0.0f};
        static float scaleBuffer = 1.f;

        if (ImGui::Begin("ObjectList", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
        {
            ImGui::SetWindowPos(ImVec2(0, 20));
            ImGui::SetWindowSize(ImVec2(io.DisplaySize.x / 7, io.DisplaySize.y / 2));
            ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), true);

            for (auto &kv : frameInfo.gameObjects)
            {
                if (ImGui::Selectable(kv.second.getName().c_str(), selectedItem == kv.first))
                {
                    selectedItem = kv.first;
                    // Copy the selected object's name to the buffer
                    strncpy(nameBuffer, kv.second.getName().c_str(), sizeof(nameBuffer) - 1);
                    nameBuffer[sizeof(nameBuffer) - 1] = '\0'; // Ensure null-termination
                    isEditingName = false;
                }
            }

            ImGui::EndChild();

            ImGui::End();
        }

        if (ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
        {
            ImGui::SetWindowPos(ImVec2(0, (io.DisplaySize.y / 2) + 20));
            ImGui::SetWindowSize(ImVec2(io.DisplaySize.x / 7, (io.DisplaySize.y / 2)));

            ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), true);
            if (selectedItem != -1)
            {
                GWGameObject &selectedObject = frameInfo.gameObjects.at(selectedItem);
                unsigned int objId = selectedObject.getId();
                std::string name = selectedObject.getName();
                glm::vec3 position = selectedObject.transform.translation;

                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "ID: %d", objId);

                positionBuffer = position;
                rotationBuffer = glm::degrees(selectedObject.transform.rotation);
                scaleBuffer = selectedObject.transform.scale;

                if (!isEditingName)
                {
                    auto nameDisplay = "Name: " + name;
                    if (ImGui::Selectable(nameDisplay.c_str(), isEditingName))
                    {
                        isEditingName = true;
                    }
                }
                else
                {
                    if (ImGui::InputText("Name: ", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        selectedObject.setName(std::string(nameBuffer));
                        isEditingName = false;
                    }
                }

                ImGui::Separator();

                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Transform");

                ImGui::Separator();

                inputPosition(positionBuffer, selectedObject);
                inputRotation(rotationBuffer, selectedObject);

                // Scale
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Scale: ");
                ImGui::SameLine();
                if (ImGui::InputFloat("##Scale", &scaleBuffer, 0.1f, 1.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    selectedObject.transform.scale = scaleBuffer;
                }
            } else {
                ImGui::Text("Select a Object");
            }
        }

        ImGui::EndChild();
        ImGui::End();
    }
}