#pragma once

#include <vulkan/vulkan.h>


#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

class IEngineServices;

namespace lve {

    class UiWrapper {
    public:
        ~UiWrapper();

        void init();
        void shutdown();
        void update(double deltaTime);
        void resize(int width, int height);
        void onMouseMove(double x, double y);
        void onMouseButton(int button, int action, int mods, double x, double y);
        void onScroll(double dx, double dy);
        void onKey(int key, int action);
        void onChar(unsigned int codepoint);

        void renderOffscreen(VkCommandBuffer cmdBuffer);
        void render(VkCommandBuffer cmdBuffer, VkRenderPass renderPass);
        void registerButtonHandler(std::string name, std::function<void()> handler);

    private:
        bool initialized_ = false;
        int width_ = 0;
        int height_ = 0;
        uint64_t frameIndex_ = 0;
        std::unordered_map<std::string, std::function<void()>> buttonHandlers_;
    };

}
