#include "GWGameObject.hpp"

namespace GWIN
{
    glm::mat4 TransformComponent::mat4()
    {
        glm::mat4 matrix(1.0f);

        glm::mat4 scaleMatrix = glm::scale(matrix, glm::vec3(scale));
        glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
        glm::mat4 translationMatrix = glm::translate(matrix, translation);

        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    GWGameObject GWGameObject::createLight(float intensity, float radius, glm::vec3 color)
    {
        GWGameObject gameObject = GWGameObject::createGameObject("PointLight");
        gameObject.color = color;
        gameObject.transform.scale = radius;
        gameObject.light = std::make_unique<LightComponent>();
        gameObject.light->lightIntensity = intensity;
        return gameObject;
    }

    GWGameObject GWGameObject::createLight(float intensity, float radius, glm::vec3 color, float cutOffAngle)
    {
        GWGameObject gameObject = GWGameObject::createGameObject("SpotLight");
        gameObject.color = color;
        gameObject.transform.scale = radius;
        gameObject.light = std::make_unique<LightComponent>();
        gameObject.light->lightIntensity = intensity;
        gameObject.light->cutOffAngle = cutOffAngle;

        return gameObject;
    }

    std::string GWGameObject::toJson() const
    {
        nlohmann::json jsonObject;

        jsonObject["id"] = id;
        jsonObject["name"] = getName();
        jsonObject["transform"] = {
            {"translation", {transform.translation.x, transform.translation.y, transform.translation.z}},
            {"rotation", {transform.rotation.x, transform.rotation.y, transform.rotation.z}},
            {"scale", transform.scale}};

        if (light)
        {
            jsonObject["color"] = {color.x, color.y, color.z};
            
            jsonObject["light"] = {
                {"intensity", light->lightIntensity},
                {"cutOffAngle", light->cutOffAngle}};
        }
        if (model)
        {
            jsonObject["modelPath"] = model->getPath();
        }
        jsonObject["textures"] = Textures;
        jsonObject["material"] = Material;

        return jsonObject.dump();
    }
}