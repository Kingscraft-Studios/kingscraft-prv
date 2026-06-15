#pragma once

#include "Core/Screen.hpp"
#include "Vulkan/Pipeline.hpp"
#include "Vulkan/Device.hpp"
#include "Vulkan/DescriptorManager.hpp"
#include "Resource/ResourceManager.hpp"
#include "UI/UiSystem.hpp"
#include <vector>
#include <memory>

namespace lve {

    class LoadingScreen : public Screen {
    public:
        LoadingScreen(Device& device, DescriptorManager& descriptorManager,
                      ResourceManager& resourceManager, VkRenderPass renderPass,
                      UiSystem& uiSystem);
        ~LoadingScreen() override;

        void init() override;
        void tick(double dt) override;
        void render(VkCommandBuffer commandBuffer) override;
        void cleanup() override;

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

        Device& device_;
        DescriptorManager& descriptorManager_;
        ResourceManager& resourceManager_;
        VkRenderPass renderPass_;
        UiSystem& uiSystem_;

        VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout_ = VK_NULL_HANDLE;
        VkDescriptorPool loadingDescriptorPool_ = VK_NULL_HANDLE;
        VkDescriptorSet loadingDescriptorSet_ = VK_NULL_HANDLE;
        std::unique_ptr<Pipeline> pipeline_;

        std::vector<char> vertShaderCode_;
        std::vector<char> fragShaderCode_;
    };

} // namespace lve
