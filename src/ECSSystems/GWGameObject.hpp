#pragma once

#include "GWModel.hpp"

//std 
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace GWIN
{
    struct TransformComponent
    {
        glm::vec3 translation{0}; // (position offset)
        glm::vec3 scale{1.f};
        glm::vec3 rotation{0};

        glm::mat4 mat4()
        {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            return glm::mat4{
                {
                    scale.x * (c1 * c3 + s1 * s2 * s3),
                    scale.x * (c2 * s3),
                    scale.x * (c1 * s2 * s3 - c3 * s1),
                    0.0f,
                },
                {
                    scale.y * (c3 * s1 * s2 - c1 * s3),
                    scale.y * (c2 * c3),
                    scale.y * (c1 * c3 * s2 + s1 * s3),
                    0.0f,
                },
                {
                    scale.z * (c2 * s1),
                    scale.z * (-s2),
                    scale.z * (c1 * c2),
                    0.0f,
                },
            {translation.x, translation.y, translation.z, 1.0f}};
        }
    };

    class GWGameObject
    {
        public:
        using id_t = unsigned int;

        static GWGameObject createGameObject()
        {
            unsigned int currentID = 0;
            return GWGameObject(currentID++);
        }

        GWGameObject(const GWGameObject &) = delete;
        GWGameObject &operator=(const GWGameObject &) = delete;
        GWGameObject(GWGameObject &&) = default;
        GWGameObject &operator=(GWGameObject &&) = default;

        id_t getId() { return id; }

        std::shared_ptr<GWModel> model{};
        glm::vec3 color{};
        TransformComponent transform{};

    private:
        GWGameObject(id_t ID)
        {
            id = ID;
        }

        id_t id;
    };
}