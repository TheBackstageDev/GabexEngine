#include "Frustum.hpp"

namespace GWIN
{
    Frustum::Frustum() {};

    void Frustum::updateFrustumPlanes(const glm::mat4 vpMatrix)
    {
        frustumPlanes[0] = glm::vec4(vpMatrix[0][3] + vpMatrix[0][0], vpMatrix[1][3] + vpMatrix[1][0], vpMatrix[2][3] + vpMatrix[2][0], vpMatrix[3][3] + vpMatrix[3][0]); // Left
        frustumPlanes[1] = glm::vec4(vpMatrix[0][3] - vpMatrix[0][0], vpMatrix[1][3] - vpMatrix[1][0], vpMatrix[2][3] - vpMatrix[2][0], vpMatrix[3][3] - vpMatrix[3][0]); // Right
        frustumPlanes[2] = glm::vec4(vpMatrix[0][3] + vpMatrix[0][1], vpMatrix[1][3] + vpMatrix[1][1], vpMatrix[2][3] + vpMatrix[2][1], vpMatrix[3][3] + vpMatrix[3][1]); // Bottom
        frustumPlanes[3] = glm::vec4(vpMatrix[0][3] - vpMatrix[0][1], vpMatrix[1][3] - vpMatrix[1][1], vpMatrix[2][3] - vpMatrix[2][1], vpMatrix[3][3] - vpMatrix[3][1]); // Top
        frustumPlanes[4] = glm::vec4(vpMatrix[0][3] + vpMatrix[0][2], vpMatrix[1][3] + vpMatrix[1][2], vpMatrix[2][3] + vpMatrix[2][2], vpMatrix[3][3] + vpMatrix[3][2]); // Near
        frustumPlanes[5] = glm::vec4(vpMatrix[0][3] - vpMatrix[0][2], vpMatrix[1][3] - vpMatrix[1][2], vpMatrix[2][3] - vpMatrix[2][2], vpMatrix[3][3] - vpMatrix[3][2]); // Far

        for (int i = 0; i < 6; ++i)
        {
            frustumPlanes[i] = glm::normalize(frustumPlanes[i]);
        }
    }

    bool Frustum::isPointInFrustum(const glm::vec3 point)
    {
        for (const auto &plane : frustumPlanes)
        {
            float distance = glm::dot(glm::vec3(plane), point) + plane.w;
            if (distance < 0)
            {
                return false;
            }
        }
        return true;
    }
}