#include "keyboard_movement_controller.hpp"

namespace GWIN
{
    void keyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, GWGameObject& gameObject)
    {
        glm::vec3 rotate{ 0 };
        if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS)
            rotate.y += 1.f;
        if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS)
            rotate.y -= 1.f;
        if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS)
            rotate.x += 1.f;
        if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS)
            rotate.x -= 1.f;

        glm::vec3 currentRotation = gameObject.transform.getRotation();

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
        {
            currentRotation += lookSpeed * dt * glm::normalize(rotate);
            currentRotation.x = glm::clamp(currentRotation.x, -1.5f, 1.5f);
            currentRotation.y = glm::mod(currentRotation.y, glm::two_pi<float>());

            gameObject.transform.rotateEuler(currentRotation);
        }

        float yaw = currentRotation.y;
        const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
        const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
        const glm::vec3 upDir{ 0.f, -1.f, 0.f };

        glm::vec3 moveDir{ 0.f };
        if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS)
            moveDir += forwardDir;
        if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS)
            moveDir -= forwardDir;
        if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS)
            moveDir += rightDir;
        if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS)
            moveDir -= rightDir;
        if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS)
            moveDir += upDir;
        if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS)
            moveDir -= upDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
        {
            gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }
}
