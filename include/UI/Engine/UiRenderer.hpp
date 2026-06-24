#pragma once

#include "Vulkan/Device.hpp"
#include "Vulkan/DescriptorManager.hpp"
#include "Vulkan/Buffer.hpp"
#include "Vulkan/Pipeline.hpp"
#include "UI/Engine/UiBatchQueue.hpp"
#include <memory>
#include <glm/glm.hpp>

namespace lve {

    class UiRenderer {
    public:
        UiRenderer(Device& device, DescriptorManager& descriptorManager, VkExtent2D extent);
        ~UiRenderer();

        UiRenderer(const UiRenderer&) = delete;
        UiRenderer& operator=(const UiRenderer&) = delete;

        void init();
        void shutdown();
        void resize(VkExtent2D extent);

        void renderOffscreen(VkCommandBuffer cmd, UiBatchQueue& batchQueue);
        void composite(VkCommandBuffer cmd);
        void setTargetRenderPass(VkRenderPass renderPass);

        void setFontAtlas(VkImageView imageView, VkSampler sampler);
        bool hasFontAtlas() const { return atlasView_ != VK_NULL_HANDLE; }

    private:
        void createRenderPass();
        void createPipeline();
        void createCompositePipeline(VkRenderPass renderPass);
        void createDescriptorSetLayouts();
        void createDescriptorPools();
        void allocateDescriptorSets();
        void createOffscreenResources();
        void destroyOffscreenResources();
        void createUniformBuffer();
        void updateUniformBuffer();
        void updateUiDescriptorSet();
        void updateCompositeDescriptorSet();

        static std::vector<char> readFile(const std::string& filename);

        Device& device_;
        DescriptorManager& descriptorManager_;
        VkExtent2D extent_;
        bool initialized_ = false;

        // Offscreen resources
        VkRenderPass offscreenRenderPass_ = VK_NULL_HANDLE;
        VkImage offscreenImage_ = VK_NULL_HANDLE;
        VkDeviceMemory offscreenMemory_ = VK_NULL_HANDLE;
        VkImageView offscreenView_ = VK_NULL_HANDLE;
        VkSampler offscreenSampler_ = VK_NULL_HANDLE;
        VkFramebuffer offscreenFramebuffer_ = VK_NULL_HANDLE;

        // Pipelines
        std::unique_ptr<Pipeline> uiPipeline_;
        VkPipeline compositePipeline_ = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
        VkPipelineLayout compositePipelineLayout_ = VK_NULL_HANDLE;

        // Descriptor set layouts
        VkDescriptorSetLayout uiDescriptorSetLayout_ = VK_NULL_HANDLE;
        VkDescriptorSetLayout compositeDescriptorSetLayout_ = VK_NULL_HANDLE;

        // Descriptor pools
        VkDescriptorPool uiDescriptorPool_ = VK_NULL_HANDLE;
        VkDescriptorPool compositeDescriptorPool_ = VK_NULL_HANDLE;

        // Descriptor sets
        VkDescriptorSet uiDescriptorSet_ = VK_NULL_HANDLE;
        VkDescriptorSet compositeDescriptorSet_ = VK_NULL_HANDLE;

        // Uniform buffer
        std::unique_ptr<Buffer> uniformBuffer_;

        // Font atlas (set externally)
        VkImageView atlasView_ = VK_NULL_HANDLE;
        VkSampler atlasSampler_ = VK_NULL_HANDLE;
        bool atlasDirty_ = false;
    };

} // namespace lve
