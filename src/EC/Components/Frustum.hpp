#pragma once

#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace GWIN
{
    class Frustum
    {
    public:
        Frustum();
        //~Frustum();

        void updateFrustumPlanes(const glm::mat4 vpMatrix);

        bool isPointInFrustum(const glm::vec3 point);
    private:
        glm::vec4 frustumPlanes[6];
    };
}