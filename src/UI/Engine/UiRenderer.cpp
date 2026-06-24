#include "UI/Engine/UiRenderer.hpp"
#include "Vulkan/Pipeline.hpp"

#include <fstream>
#include <cstddef>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>

namespace lve {

    UiRenderer::UiRenderer(Device& device, DescriptorManager& descriptorManager, VkExtent2D extent)
        : device_(device), descriptorManager_(descriptorManager), extent_(extent) {}

    UiRenderer::~UiRenderer() {
        shutdown();
    }

    void UiRenderer::init() {
        createRenderPass();
        createDescriptorSetLayouts();
        createDescriptorPools();
        createUniformBuffer();
        createOffscreenResources();
        allocateDescriptorSets();
        createPipeline();
        updateUiDescriptorSet();
        updateCompositeDescriptorSet();
        updateUniformBuffer();
        initialized_ = true;
    }

    void UiRenderer::shutdown() {
        if (!initialized_) return;
        uiPipeline_.reset();
        if (compositePipeline_ != VK_NULL_HANDLE) {
            vkDestroyPipeline(device_.device(), compositePipeline_, nullptr);
            compositePipeline_ = VK_NULL_HANDLE;
        }
        destroyOffscreenResources();
        if (compositePipelineLayout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_.device(), compositePipelineLayout_, nullptr);
            compositePipelineLayout_ = VK_NULL_HANDLE;
        }
        if (pipelineLayout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_.device(), pipelineLayout_, nullptr);
            pipelineLayout_ = VK_NULL_HANDLE;
        }
        if (uiDescriptorSetLayout_ != VK_NULL_HANDLE) {
            descriptorManager_.destroyLayout(uiDescriptorSetLayout_);
            uiDescriptorSetLayout_ = VK_NULL_HANDLE;
        }
        if (compositeDescriptorSetLayout_ != VK_NULL_HANDLE) {
            descriptorManager_.destroyLayout(compositeDescriptorSetLayout_);
            compositeDescriptorSetLayout_ = VK_NULL_HANDLE;
        }
        if (uiDescriptorPool_ != VK_NULL_HANDLE) {
            descriptorManager_.destroyPool(uiDescriptorPool_);
            uiDescriptorPool_ = VK_NULL_HANDLE;
        }
        if (compositeDescriptorPool_ != VK_NULL_HANDLE) {
            descriptorManager_.destroyPool(compositeDescriptorPool_);
            compositeDescriptorPool_ = VK_NULL_HANDLE;
        }
        if (offscreenRenderPass_ != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device_.device(), offscreenRenderPass_, nullptr);
            offscreenRenderPass_ = VK_NULL_HANDLE;
        }
        uniformBuffer_.reset();
        initialized_ = false;
    }

    void UiRenderer::resize(VkExtent2D extent) {
        if (!initialized_) return;
        destroyOffscreenResources();
        extent_ = extent;
        createOffscreenResources();
        updateCompositeDescriptorSet();
        updateUniformBuffer();
    }

    void UiRenderer::renderOffscreen(VkCommandBuffer cmd, UiBatchQueue& batchQueue) {
        if (!initialized_ || extent_.width == 0 || extent_.height == 0) return;
        if (atlasView_ == VK_NULL_HANDLE) return;

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.image = offscreenImage_;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        vkCmdPipelineBarrier(cmd,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkClearValue clear = {{{0.0f, 0.0f, 0.0f, 0.0f}}};
        VkRenderPassBeginInfo passInfo{};
        passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        passInfo.renderPass = offscreenRenderPass_;
        passInfo.framebuffer = offscreenFramebuffer_;
        passInfo.renderArea = {{0, 0}, extent_};
        passInfo.clearValueCount = 1;
        passInfo.pClearValues = &clear;
        vkCmdBeginRenderPass(cmd, &passInfo, VK_SUBPASS_CONTENTS_INLINE);

        uiPipeline_->bind(cmd);

        VkViewport viewport{0.0f, 0.0f,
            static_cast<float>(extent_.width), static_cast<float>(extent_.height),
            0.0f, 1.0f};
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{{0, 0}, extent_};
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        if (atlasDirty_) {
            updateUiDescriptorSet();
            atlasDirty_ = false;
        }

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout_, 0, 1, &uiDescriptorSet_, 0, nullptr);

        batchQueue.flush(cmd);

        vkCmdEndRenderPass(cmd);
    }

    void UiRenderer::composite(VkCommandBuffer cmd) {
        if (!initialized_ || extent_.width == 0 || extent_.height == 0) return;

        VkViewport viewport{0.0f, 0.0f,
            static_cast<float>(extent_.width), static_cast<float>(extent_.height),
            0.0f, 1.0f};
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{{0, 0}, extent_};
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, compositePipeline_);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                compositePipelineLayout_, 0, 1, &compositeDescriptorSet_, 0, nullptr);
        vkCmdDraw(cmd, 3, 1, 0, 0);
    }

    void UiRenderer::setTargetRenderPass(VkRenderPass renderPass) {
        if (compositePipeline_ != VK_NULL_HANDLE) {
            vkDestroyPipeline(device_.device(), compositePipeline_, nullptr);
            compositePipeline_ = VK_NULL_HANDLE;
        }
        if (compositePipelineLayout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_.device(), compositePipelineLayout_, nullptr);
            compositePipelineLayout_ = VK_NULL_HANDLE;
        }
        createCompositePipeline(renderPass);
    }

    void UiRenderer::setFontAtlas(VkImageView imageView, VkSampler sampler) {
        atlasView_ = imageView;
        atlasSampler_ = sampler;
        atlasDirty_ = true;
    }

    void UiRenderer::createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;

        VkSubpassDependency dependencies[2]{};
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = dependencies;

        if (vkCreateRenderPass(device_.device(), &renderPassInfo, nullptr, &offscreenRenderPass_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create offscreen render pass!");
        }
    }

    void UiRenderer::createDescriptorSetLayouts() {
        std::vector<VkDescriptorSetLayoutBinding> uiBindings(2);
        uiBindings[0].binding = 0;
        uiBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uiBindings[0].descriptorCount = 1;
        uiBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        uiBindings[1].binding = 1;
        uiBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        uiBindings[1].descriptorCount = 1;
        uiBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        uiDescriptorSetLayout_ = descriptorManager_.createLayout(uiBindings);

        std::vector<VkDescriptorSetLayoutBinding> compBindings(1);
        compBindings[0].binding = 0;
        compBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        compBindings[0].descriptorCount = 1;
        compBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        compositeDescriptorSetLayout_ = descriptorManager_.createLayout(compBindings);
    }

    void UiRenderer::createDescriptorPools() {
        std::vector<VkDescriptorPoolSize> uiPoolSizes(2);
        uiPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uiPoolSizes[0].descriptorCount = 1;
        uiPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        uiPoolSizes[1].descriptorCount = 1;

        uiDescriptorPool_ = descriptorManager_.createPool(uiPoolSizes, 1);

        std::vector<VkDescriptorPoolSize> compPoolSizes(1);
        compPoolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        compPoolSizes[0].descriptorCount = 1;

        compositeDescriptorPool_ = descriptorManager_.createPool(compPoolSizes, 1);
    }

    void UiRenderer::allocateDescriptorSets() {
        uiDescriptorSet_ = descriptorManager_.allocateSet(uiDescriptorPool_, uiDescriptorSetLayout_);
        compositeDescriptorSet_ = descriptorManager_.allocateSet(compositeDescriptorPool_, compositeDescriptorSetLayout_);
    }

    void UiRenderer::createPipeline() {
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &uiDescriptorSetLayout_;

        if (vkCreatePipelineLayout(device_.device(), &layoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create UI pipeline layout!");
        }

        auto vertCode = readFile("resources/shaders/ui.vert.spv");
        auto fragCode = readFile("resources/shaders/ui.frag.spv");

        PipelineConfigInfo configInfo{};
        Pipeline::defaultPipelineConfigInfo(configInfo);

        VkVertexInputBindingDescription bindingDesc{};
        bindingDesc.binding = 0;
        bindingDesc.stride = sizeof(UiVertex);
        bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::vector<VkVertexInputAttributeDescription> attributeDescs(3);
        attributeDescs[0].binding = 0;
        attributeDescs[0].location = 0;
        attributeDescs[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescs[0].offset = static_cast<uint32_t>(offsetof(UiVertex, pos));

        attributeDescs[1].binding = 0;
        attributeDescs[1].location = 1;
        attributeDescs[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescs[1].offset = static_cast<uint32_t>(offsetof(UiVertex, uv));

        attributeDescs[2].binding = 0;
        attributeDescs[2].location = 2;
        attributeDescs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescs[2].offset = static_cast<uint32_t>(offsetof(UiVertex, color));

        configInfo.bindingDescriptions = {bindingDesc};
        configInfo.attributeDescriptions = {attributeDescs.begin(), attributeDescs.end()};

        configInfo.blendAttachmentState.blendEnable = VK_TRUE;
        configInfo.blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        configInfo.blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        configInfo.blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        configInfo.blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        configInfo.blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        configInfo.blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

        configInfo.depthStencilInfo.depthTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_FALSE;

        configInfo.renderPass = offscreenRenderPass_;
        configInfo.pipelineLayout = pipelineLayout_;

        uiPipeline_ = std::make_unique<Pipeline>(device_, vertCode, fragCode, configInfo);
    }

    void UiRenderer::createCompositePipeline(VkRenderPass renderPass) {
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &compositeDescriptorSetLayout_;

        if (vkCreatePipelineLayout(device_.device(), &layoutInfo, nullptr, &compositePipelineLayout_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create composite pipeline layout!");
        }

        auto vertCode = readFile("resources/shaders/composite.vert.spv");
        auto fragCode = readFile("resources/shaders/composite.frag.spv");

        // Create shader modules manually
        VkShaderModule vertModule, fragModule;
        {
            VkShaderModuleCreateInfo ci{};
            ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            ci.codeSize = vertCode.size();
            ci.pCode = reinterpret_cast<const uint32_t*>(vertCode.data());
            if (vkCreateShaderModule(device_.device(), &ci, nullptr, &vertModule) != VK_SUCCESS)
                throw std::runtime_error("failed to create composite vert shader module!");
        }
        {
            VkShaderModuleCreateInfo ci{};
            ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            ci.codeSize = fragCode.size();
            ci.pCode = reinterpret_cast<const uint32_t*>(fragCode.data());
            if (vkCreateShaderModule(device_.device(), &ci, nullptr, &fragModule) != VK_SUCCESS)
                throw std::runtime_error("failed to create composite frag shader module!");
        }

        VkPipelineShaderStageCreateInfo stages[2]{};
        stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = vertModule;
        stages[0].pName = "main";
        stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = fragModule;
        stages[1].pName = "main";

        // No vertex input (uses gl_VertexIndex)
        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisample{};
        multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.blendEnable = VK_TRUE;
        blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &blendAttachment;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_FALSE;
        depthStencil.depthWriteEnable = VK_FALSE;

        VkDynamicState dynStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynStates;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = stages;
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisample;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = compositePipelineLayout_;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;

        if (vkCreateGraphicsPipelines(device_.device(), VK_NULL_HANDLE, 1,
                                      &pipelineInfo, nullptr, &compositePipeline_) != VK_SUCCESS) {
            vkDestroyShaderModule(device_.device(), vertModule, nullptr);
            vkDestroyShaderModule(device_.device(), fragModule, nullptr);
            throw std::runtime_error("failed to create composite pipeline!");
        }

        vkDestroyShaderModule(device_.device(), vertModule, nullptr);
        vkDestroyShaderModule(device_.device(), fragModule, nullptr);
    }

    void UiRenderer::createUniformBuffer() {
        uniformBuffer_ = std::make_unique<Buffer>(
            device_, sizeof(glm::mat4),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    void UiRenderer::updateUniformBuffer() {
        glm::mat4 proj = glm::ortho(
            0.0f, static_cast<float>(extent_.width),
            static_cast<float>(extent_.height), 0.0f,
            -1.0f, 1.0f);
        uniformBuffer_->write(&proj, 0, sizeof(glm::mat4));
    }

    void UiRenderer::updateUiDescriptorSet() {
        if (atlasView_ == VK_NULL_HANDLE) return;
        // Write UBO binding
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffer_->getHandle();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(glm::mat4);

        VkWriteDescriptorSet uboWrite{};
        uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uboWrite.dstSet = uiDescriptorSet_;
        uboWrite.dstBinding = 0;
        uboWrite.descriptorCount = 1;
        uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboWrite.pBufferInfo = &bufferInfo;

        // Write font atlas binding
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = atlasView_;
        imageInfo.sampler = atlasSampler_;

        VkWriteDescriptorSet imageWrite{};
        imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imageWrite.dstSet = uiDescriptorSet_;
        imageWrite.dstBinding = 1;
        imageWrite.descriptorCount = 1;
        imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageWrite.pImageInfo = &imageInfo;

        VkWriteDescriptorSet writes[] = {uboWrite, imageWrite};
        descriptorManager_.updateDescriptorSets(
            std::vector<VkWriteDescriptorSet>(writes, writes + 2));
    }

    void UiRenderer::updateCompositeDescriptorSet() {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = offscreenView_;
        imageInfo.sampler = offscreenSampler_;

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = compositeDescriptorSet_;
        write.dstBinding = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = &imageInfo;

        descriptorManager_.updateDescriptorSets(std::vector<VkWriteDescriptorSet>{write});
    }

    void UiRenderer::createOffscreenResources() {
        device_.createImage(
            extent_.width, extent_.height,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            offscreenImage_,
            offscreenMemory_);

        offscreenView_ = device_.createImageView(
            offscreenImage_, VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_ASPECT_COLOR_BIT);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        if (vkCreateSampler(device_.device(), &samplerInfo, nullptr, &offscreenSampler_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create offscreen sampler!");
        }

        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = offscreenRenderPass_;
        fbInfo.attachmentCount = 1;
        fbInfo.pAttachments = &offscreenView_;
        fbInfo.width = extent_.width;
        fbInfo.height = extent_.height;
        fbInfo.layers = 1;

        if (vkCreateFramebuffer(device_.device(), &fbInfo, nullptr, &offscreenFramebuffer_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create offscreen framebuffer!");
        }

        // Transition image from UNDEFINED to SHADER_READ_ONLY_OPTIMAL so the
        // first renderOffscreen barrier works correctly.
        VkCommandBuffer cmd = device_.beginSingleTimeCommands();
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.image = offscreenImage_;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmd,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &barrier);
        device_.endSingleTimeCommands(cmd);
    }

    void UiRenderer::destroyOffscreenResources() {
        if (offscreenFramebuffer_ != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device_.device(), offscreenFramebuffer_, nullptr);
            offscreenFramebuffer_ = VK_NULL_HANDLE;
        }
        if (offscreenSampler_ != VK_NULL_HANDLE) {
            vkDestroySampler(device_.device(), offscreenSampler_, nullptr);
            offscreenSampler_ = VK_NULL_HANDLE;
        }
        if (offscreenView_ != VK_NULL_HANDLE) {
            vkDestroyImageView(device_.device(), offscreenView_, nullptr);
            offscreenView_ = VK_NULL_HANDLE;
        }
        if (offscreenImage_ != VK_NULL_HANDLE) {
            vkDestroyImage(device_.device(), offscreenImage_, nullptr);
            offscreenImage_ = VK_NULL_HANDLE;
        }
        if (offscreenMemory_ != VK_NULL_HANDLE) {
            vkFreeMemory(device_.device(), offscreenMemory_, nullptr);
            offscreenMemory_ = VK_NULL_HANDLE;
        }
    }

    std::vector<char> UiRenderer::readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file: " + filename);
        }
        size_t fileSize = static_cast<size_t>(file.tellg());
        file.seekg(0);
        std::vector<char> buffer(fileSize);
        if (!file.read(buffer.data(), fileSize)) {
            throw std::runtime_error("failed to read file: " + filename);
        }
        return buffer;
    }

} // namespace lve
