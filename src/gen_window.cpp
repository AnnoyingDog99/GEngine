#include "gen_window.hpp"
#include <stdexcept>

namespace gen
{

    GenWindow::GenWindow(int width, int height, std::string name) : width{width}, height{height}, windowName{name}
    {
        initWindow();
    }

    GenWindow::~GenWindow()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void GenWindow::initWindow()
    {
        glfwInit();
        // don't create openGL context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // don't resize after creation
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferresizeCallback);
    }

    void GenWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void GenWindow::framebufferresizeCallback(GLFWwindow *window, int width, int height)
    {
        auto genWindow = reinterpret_cast<GenWindow *>(glfwGetWindowUserPointer(window));
        genWindow->frameBufferResized = true;
        genWindow->width = width;
        genWindow->height = height;
    }
}