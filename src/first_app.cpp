#include "first_app.hpp"

namespace gen{
    void FirstApp::run(){
        while(!genWindow.shouldClose()){
            glfwPollEvents();
        }
    }
}