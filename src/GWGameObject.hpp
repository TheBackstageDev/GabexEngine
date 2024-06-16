#pragma once

#include "GWModel.hpp"

//std 
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace GWIN
{
    struct Transform2D
    {
        glm::vec2 translation{}; // (position offset)
        glm::vec2 scale{1.f, 1.f};
        float rotation;

        glm::mat2 mat2()
        {
            const float s = glm::sin(rotation);
            const float c = glm::cos(rotation);
            glm::mat2 rotMatrix{{c, s}, {-s, c}};

            glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
            return rotMatrix * scaleMat;
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
        Transform2D transform2d{};

    private:
        GWGameObject(id_t ID)
        {
            id = ID;
        }

        id_t id;
    };
}