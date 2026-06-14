#pragma once

#include "Vulkan/Window.hpp"
#include "Vulkan/Pipeline.hpp"
#include "Vulkan/Device.hpp"
#include "Vulkan/Swapchain.hpp"
#include "Vulkan/Model.hpp"
#include "Vulkan/Texture.hpp"
#include <memory>
#include <vector>

#include "UI/UiSystem.hpp"


namespace lve {
    enum GameState {
        LOADING,
        MAIN_MENU
    };

    class App {
    public:
        static constexpr int WIDTH = 900;
        static constexpr int HEIGHT = 650;

        App();
        ~App();
        App(const App &) = delete;
        App &operator=(const App &) = delete;

        void run();

        enum class RenderState {
            Running,
            Paused,
            Rebuilding
        };

        void pauseRenderer();
        void resumeRenderer();

    private:
        void loadModels();
        void createDescriptorSetLayout();
        void createLoadingDescriptorPool();
        void createLoadingDescriptorSet();
        void updateLoadingDescriptorSet();
        void createPipelineLayout();
        void createCommandBuffers();
        void freeCommandBuffers();
        void createPipeline();
        void drawFrame();
        void recreateSwapChain();
        void recordCommandBuffer(int imageIndex);

        VkImageView getTextureImageView() {
            return texture->getImageView();
        }

        VkSampler getTextureSampler() {
            return texture->getSampler();
        }

        Window window{WIDTH, HEIGHT, "Kingscraft"};
        Device device{window};
        std::unique_ptr<SwapChain> swapchain;
        std::unique_ptr<Pipeline> pipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        std::unique_ptr<Model> model;
        std::unique_ptr<Texture> texture;
        bool requestSwapchainRecreate = false;
        VkExtent2D lastExtent{0, 0};
        VkDescriptorPool loadingDescriptorPool;
        VkDescriptorSet loadingDescriptorSet;
        VkDescriptorSetLayout descriptorSetLayout;
        RenderState renderState = RenderState::Running;

        std::unique_ptr<UiSystem> uiSystem;

    };

}  // namespace lve
