#pragma once

#include "gen_model.hpp"

// glm
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace gen
{
    // should be changed out later by an entity component system

    struct TransformComponent
    {
        glm::vec3 translation{};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation{};

        // Matrix corresponds to tranlate * Ry * Rx * Rz * scale transformation
        // Rotation convention uses tait-bryan angles with axis order Y(1), X(2), Z(3) (left to right: Intrinisic, right to left: extrinsic)
        // (Maybe replace with quaternions?)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };

    struct PointLightComponent
    {
        float lightIntensity = 1.f;
    };

    class GenGameObject
    {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, GenGameObject>;

        static GenGameObject createGameObject()
        {
            static id_t currentId = 0;
            return GenGameObject{currentId++};
        }

        static GenGameObject makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

        GenGameObject(const GenGameObject &) = delete;
        GenGameObject &operator=(const GenGameObject &) = delete;
        GenGameObject(GenGameObject &&) = default;
        GenGameObject &operator=(GenGameObject &&) = default;

        id_t getId() const
        {
            return id;
        }

        glm::vec3 color{};
        TransformComponent transform{};

        // optional pointer components
        std::shared_ptr<GenModel> model{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;

    private:
        GenGameObject(id_t objId) : id{objId} {}

        id_t id;
    };
}