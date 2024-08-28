#pragma once

#include "GWModel.hpp"
#include <array>
//std 
#include <memory>
#include <unordered_map>
#include "../json.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

namespace GWIN
{
    struct TransformComponent
    {
        glm::vec3 translation{0}; // (position offset)
        float scale{1.f};
        glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};

        void rotate(glm::vec3 axis, float angle)
        {
            glm::quat deltaRotation = glm::angleAxis(glm::radians(angle), glm::normalize(axis));

            rotation = glm::normalize(deltaRotation * rotation); // Normalize the quaternion to avoid drift
        }

        void rotateEuler(glm::vec3 angles)
        {
            rotation = glm::normalize(glm::quat(glm::radians(angles)));
        }

        glm::vec3 getRotation() { return glm::degrees(glm::eulerAngles(rotation)); };

        glm::mat4 mat4();
    };

    struct LightComponent
    {
        float lightIntensity = 1.f;
        float cutOffAngle = 0.0f;
    };

    class GWGameObject
    {
        public:
        using id_t = unsigned int;
        using map = std::unordered_map<id_t, GWGameObject>;

        static GWGameObject createGameObject(const std::string name)
        {
            static id_t currentID = 0;
            return GWGameObject(currentID++, name);
        }

        static GWGameObject createGameObject(const std::string name, id_t id)
        {
            return GWGameObject(id, name);
        }

        static GWGameObject createLight(float intensity, float radius, glm::vec3 color);
        static GWGameObject createLight(float intensity, float radius, glm::vec3 color, float cutOffAngle);

        GWGameObject(const GWGameObject &) = delete;
        GWGameObject &operator=(const GWGameObject &) = delete;
        GWGameObject(GWGameObject &&) = default;
        GWGameObject &operator=(GWGameObject &&) = default;

        id_t getId() const { return id; }
        std::string getName() const { return Objname; };
        void setName(std::string& newName) { Objname = newName; };

        glm::vec3 color{};
        TransformComponent transform{};

        //opitional components
        std::array<uint32_t, 6> Textures; // ID of the textures
        uint32_t Material = 0; //ID of the material

        std::shared_ptr<GWModel> model{};
        std::unique_ptr<LightComponent> light = nullptr;

        std::string toJson() const;

    private:
        static id_t currentID;
        
        GWGameObject(id_t ID, const std::string name) : Objname(name)
        {
            id = ID;
        }

        std::string Objname;
        id_t id;
    };
}