#include "Vulkan/Window.hpp"
#include "Vulkan/Texture.hpp"

// std
#include <stdexcept>

#include "stb_image.h"
#include "Bus/BusUtil.hpp"
#include "Bus/Message.hpp"
#include "Threads/IO.hpp"
#include "Threads/Logger.hpp"

namespace lve {

    Window::Window(int w, int h, const std::string& name) : width{w}, height{h}, windowName{name} {
        initWindow();
    }

    Window::~Window() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Window::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

        GLFWimage icon[1];
        int channels;

        auto iconData = IO::Get().readFile("resources/textures/logo/Kingscraft-Logo.png");
        icon[0].pixels = stbi_load_from_memory(
            reinterpret_cast<const stbi_uc*>(iconData.data()),
            iconData.size(),
            &icon[0].width, &icon[0].height, &channels, 4);

        if (icon[0].pixels) {
            glfwSetWindowIcon(window, 1, icon);
            stbi_image_free(icon[0].pixels);
        } else {
            BusUtil::structure(SType::Send, ThreadName::Engine, []() {
                   Logger::Get().log(LogLevel::WARN, ThreadName::Renderer, "Failed to load window icon!");
               });
        }

        glfwGetWindowPos(window, &windowedX, &windowedY);
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);

        monitor = glfwGetPrimaryMonitor();
        videoMode = glfwGetVideoMode(monitor);


        glfwSetWindowUserPointer(window, this);

        glfwSetCursorPosCallback(window, [](GLFWwindow* w, double xpos, double ypos) {
    auto win = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (win) {
        // --- THE MISSING PIECE: Save the position! ---
        win->lastX = xpos;
        win->lastY = ypos;

        if (win->mouseMovementCallback) {
            win->mouseMovementCallback(xpos, ypos);
        }
    }
});

        glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
        auto lveWin = static_cast<Window*>(glfwGetWindowUserPointer(w));
        if (lveWin && lveWin->mouseButtonCallback) {
            lveWin->mouseButtonCallback(button, action, mods);
        }
    });

        glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
            auto win = static_cast<Window*>(glfwGetWindowUserPointer(w));
            if (win && win->keyCallback) {
                win->keyCallback(key, scancode, action, mods);
            }
        });

        glfwSetCharCallback(window, [](GLFWwindow* w, unsigned int codepoint) {
            auto win = static_cast<Window*>(glfwGetWindowUserPointer(w));
            if (win && win->charCallback) {
                win->charCallback(codepoint);
            }
        });

        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto lveWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (!lveWindow) return;

        // Prevent resizing if still in the loading screen
        if (lveWindow->getLoading()) {
            // Reset to fixed size (you can hardcode or use existing values)
            glfwSetWindowSize(window, 800, 600);
            return;
        }

        lveWindow->framebufferResized = true;
        lveWindow->width = width;
        lveWindow->height = height;
    }

    void Window::toggleFullscreen() {
        if (!isFullscreen) {
            // Save the current window position and size
            glfwGetWindowPos(window, &windowedX, &windowedY);
            glfwGetWindowSize(window, &windowedWidth, &windowedHeight);

            // Switch to fullscreen on the primary monitor
            glfwSetWindowMonitor(window, monitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
            isFullscreen = true;
        } else {
            // Restore to windowed mode
            glfwSetWindowMonitor(window, nullptr, windowedX, windowedY, windowedWidth, windowedHeight, 0);
            isFullscreen = false;
        }
    }

    void Window::processInput() {
        // Input is now handled by KeyBindHandler
    }



}  // namespace lve