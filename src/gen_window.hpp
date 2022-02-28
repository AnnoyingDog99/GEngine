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

        VkExtent2D getExtend()
        {
            return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        void initWindow();

        const int width;
        const int height;

        std::string windowName;
        GLFWwindow *window;
    };
}