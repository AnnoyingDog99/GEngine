#pragma once

// glm
#define GLM_FORCE_RADIANS           // glm functions will except values in radians, not degrees
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm functions will expect depth values to be in range [0, 1]
#include <glm/glm.hpp>

namespace gen
{
    class GenCamera
    {
    public:
        void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
        void setPerspectiveProjection(float fovy, float aspectRatio, float near, float far);

        const glm::mat4 &getProjection() const
        {
            return projectionMatrix;
        }

    private:
        glm::mat4 projectionMatrix{1.f};
    };
}