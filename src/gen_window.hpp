#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
namespace gen
{
    class GenWindow
    {

    public:
        GenWindow(int width, int height, std::string name);
        ~GenWindow();

        // delete copy constructor and copy operator from window class to prevent dangling pointer
        GenWindow(const GenWindow &) = delete;
        GenWindow &operator=(const GenWindow &) = delete;

        bool shouldClose()
        {
            return glfwWindowShouldClose(window);
        }

        VkExtent2D getExtent()
        {
            return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        }

        bool wasWindowResized()
        {
            return frameBufferResized;
        }

        void resetWindowResizedFlag()
        {
            frameBufferResized = false;
        }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        static void framebufferresizeCallback(GLFWwindow *window, int width, int height);
        void initWindow();

        int width;
        int height;
        bool frameBufferResized = false;

        std::string windowName;
        GLFWwindow *window;
    };
}