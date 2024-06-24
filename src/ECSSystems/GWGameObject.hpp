#pragma once

#include "GWModel.hpp"

//std 
#include <memory>
#include <unordered_map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace GWIN
{
    struct TransformComponent
    {
        glm::vec3 translation{0}; // (position offset)
        float scale{1.f};
        glm::vec3 rotation{0};

        glm::mat4 mat4();
    };

    struct PointlightComponent
    {
        float lightIntensity = 1.f;
    };

    class GWGameObject
    {
        public:
        using id_t = unsigned int;
        using map = std::unordered_map<id_t, GWGameObject>;

        static GWGameObject createGameObject()
        {
            static id_t currentID = 0;
            return GWGameObject(currentID++);
        }

        static GWGameObject createPointLight(float intensity, float radius, glm::vec3 color);

        GWGameObject(const GWGameObject &) = delete;
        GWGameObject &operator=(const GWGameObject &) = delete;
        GWGameObject(GWGameObject &&) = default;
        GWGameObject &operator=(GWGameObject &&) = default;

        id_t getId() { return id; }

        glm::vec3 color{};
        TransformComponent transform{};

        //opitional components
        std::shared_ptr<GWModel> model{};
        std::unique_ptr<PointlightComponent> pointLight = nullptr;

    private:
        GWGameObject(id_t ID)
        {
            id = ID;
        }

        id_t id;
    };
}