#include "DebugVisuals.hpp"

namespace GWIN
{
    ////// Debug Element //////

    DebugElement::DebugElement(std::vector<DebugVertex> &vertices, uint32_t &id) : vertices(vertices), id(id)
    {
    }

    void DebugElement::drawElement(DebugElementInfo &newInfo)
    {
        ImDrawList* drawList = DebugVisuals::getDrawList();
        transform.translation = newInfo.wPosition;
        transform.scale = newInfo.scale;
    }

    ////// Debug Visuals //////

    void DebugVisuals::draw(FrameInfo& frameInfo) 
    {
        if (drawList == nullptr)
            return;

        for (auto& [id, element] : debugMeshes)
        {
            
        }
    }

    void DebugVisuals::createCone(GWGameObject &gameObject) {}
    void DebugVisuals::createSphere(GWGameObject &gameObject) {}
    void DebugVisuals::createParalelepiped(GWGameObject& gameObject) 
    {

    }
}