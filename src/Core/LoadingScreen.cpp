#include "Core/LoadingScreen.hpp"
#include <cassert>

namespace lve {

    LoadingScreen::LoadingScreen(Device& device, DescriptorManager& descriptorManager,
                                 ResourceManager& resourceManager, VkRenderPass renderPass,
                                 UiSystem& uiSystem)
        : device_(device), descriptorManager_(descriptorManager),
          resourceManager_(resourceManager), renderPass_(renderPass),
          uiSystem_(uiSystem) {}

    LoadingScreen::~LoadingScreen() {
        cleanup();
    }

    void LoadingScreen::init() {
        {
            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = 0;
            samplerLayoutBinding.descriptorCount = 1;
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            descriptorSetLayout_ = descriptorManager_.createLayout({samplerLayoutBinding});
        }

        {
            VkDescriptorPoolSize poolSize{};
            poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSize.descriptorCount = 1;
            loadingDescriptorPool_ = descriptorManager_.createPool({poolSize}, 1);
        }

        createPipelineLayout();

        resourceManager_.loadTexture(
            "resources/textures/logo/Kingscraft-Logo.png",
            [this](Texture* loadingTexture) {
                loadingDescriptorSet_ = descriptorManager_.allocateSet(loadingDescriptorPool_, descriptorSetLayout_);
                descriptorManager_.writeImageDescriptor(loadingDescriptorSet_, loadingTexture->getImageView(), loadingTexture->getSampler());
            });

        auto shadersReady = std::make_shared<int>(0);
        resourceManager_.loadShader(
            "resources/shaders/simple_shader.vert.spv",
            [this, shadersReady](const std::vector<char>& data) {
                vertShaderCode_.assign(data.begin(), data.end());
                if (++(*shadersReady) == 2)
                    createPipeline(renderPass_);
            });
        resourceManager_.loadShader(
            "resources/shaders/simple_shader.frag.spv",
            [this, shadersReady](const std::vector<char>& data) {
                fragShaderCode_.assign(data.begin(), data.end());
                if (++(*shadersReady) == 2)
                    createPipeline(renderPass_);
            });
    }

    void LoadingScreen::tick(double dt) {}

    void LoadingScreen::render(const FrameContext& ctx) {}

    void LoadingScreen::onRenderPassChanged(VkRenderPass renderPass) {
        renderPass_ = renderPass;
        if (!vertShaderCode_.empty() && !fragShaderCode_.empty()) {
            pipeline_.reset();
            createPipeline(renderPass_);
        }
    }

    void LoadingScreen::cleanup() {
        pipeline_.reset();

        if (pipelineLayout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_.device(), pipelineLayout_, nullptr);
            pipelineLayout_ = VK_NULL_HANDLE;
        }

        if (loadingDescriptorPool_ != VK_NULL_HANDLE) {
            descriptorManager_.destroyPool(loadingDescriptorPool_);
            loadingDescriptorPool_ = VK_NULL_HANDLE;
        }

        if (descriptorSetLayout_ != VK_NULL_HANDLE) {
            descriptorManager_.destroyLayout(descriptorSetLayout_);
            descriptorSetLayout_ = VK_NULL_HANDLE;
        }
    }

    void LoadingScreen::createPipelineLayout() {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;

        if (vkCreatePipelineLayout(device_.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void LoadingScreen::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout_ != nullptr && "Cannot create pipeline before pipeline layout");
        assert(!vertShaderCode_.empty() && !fragShaderCode_.empty() && "Cannot create pipeline before shaders loaded");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout_;

        pipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
        pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS;
        pipelineConfig.depthStencilInfo.stencilTestEnable = VK_FALSE;

        pipelineConfig.blendAttachmentState.blendEnable = VK_TRUE;

        pipelineConfig.blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        pipelineConfig.blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        pipelineConfig.blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;

        pipelineConfig.blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        pipelineConfig.blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        pipelineConfig.blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

        pipeline_ = std::make_unique<Pipeline>(
            device_,
            vertShaderCode_,
            fragShaderCode_,
            pipelineConfig
        );
    }

} // namespace lve
