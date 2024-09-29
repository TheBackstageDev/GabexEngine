#pragma once

#include "../GWindow.hpp"
#include "../GWDevice.hpp"
#include "../GWPipeLine.hpp"
#include "../EC/GWFrameInfo.hpp"
#include "../EC/GWGameObject.hpp"
#include "../EC/GWCamera.hpp"

// std
#include <memory>
#include <vector>
#include <stdexcept>

namespace GWIN
{
    class LightSystem
    {
    public:
        LightSystem() {};
        ~LightSystem() {};

        LightSystem(const LightSystem &) = delete;
        LightSystem &operator=(const LightSystem &) = delete;

        void update(FrameInfo& frameInfo, GlobalUbo& Ubo);

        void calculateLightMatrix(std::array<glm::mat4, 6> &matrix, float aspect, float Near, float Far);
        glm::mat4 calculateDirectionalLightMatrix(glm::vec3 cameraPosition, glm::vec3 lightRotation);
    };
}