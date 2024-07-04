#include "ObjectList.hpp"

namespace GWIN
{
    GWObjectList::GWObjectList()
    {

    }

    GWObjectList::~GWObjectList()
    {

    }

    void GWObjectList::Draw()
    {
        if (ImGui::Begin("ObjectList", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
        {

        }

        ImGui::End();
    }
}