#pragma once

#include "Components/Frustum.hpp"

#include <string>
#include "../json.hpp"

namespace GWIN
{
    class GWCamera
    {
    public:

    GWCamera() { static uint32_t last_id = 0; id = ++last_id - 1; }

    void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);

    void setPerspectiveProjection(float fovy, float aspect, float near, float far);

    void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
    void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
    void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

    void setViewerObject(uint32_t viewerId) { viewerObject = viewerId; };

    const uint32_t getViewerObject() { return viewerObject; }
    const glm::mat4& getProjection() const { return projectionMatrix; };
    const glm::mat4& getView() const { return viewMatrix; };
    const glm::mat4& getInverseView() const { return inverseViewMatrix; };
    const float getNearClip() const { return nearPlane; }
    const float getFarClip() const { return nearPlane; }
    const float getFovy() const { return fovy; }

    uint32_t getId() { return id; }

    std::string toJson() const;

    void updateFrustumPlanes() { cameraFrustum.updateFrustumPlanes(viewMatrix * projectionMatrix); }
    bool isPointInFrustum(const glm::vec3 point) { return cameraFrustum.isPointInFrustum(point); }

    private:
    
        glm::mat4 projectionMatrix{1.f};
        glm::mat4 viewMatrix{1.f};
        glm::mat4 inverseViewMatrix{1.f};

        float nearPlane;
        float farPlane;
        float fovy;

        Frustum cameraFrustum{};

        uint32_t viewerObject = 0; //None
        uint32_t id{0};

        glm::vec4 frustumPlanes[6];
    };
    
} // namespace GWIN
