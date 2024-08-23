#include "GWGameObject.hpp"

namespace GWIN
{
    glm::mat4 TransformComponent::mat4()
    {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        return glm::mat4{
            {
                scale * (c1 * c3 + s1 * s2 * s3),
                scale * (c2 * s3),
                scale * (c1 * s2 * s3 - c3 * s1),
                0.0f,
            },
            {
                scale * (c3 * s1 * s2 - c1 * s3),
                scale * (c2 * c3),
                scale * (c1 * c3 * s2 + s1 * s3),
                0.0f,
            },
            {
                scale * (c2 * s1),
                scale * (-s2),
                scale * (c1 * c2),
                0.0f,
            },
        {translation.x, translation.y, translation.z, 1.0f}};
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