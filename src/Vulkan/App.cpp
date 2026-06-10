#include "Vulkan/App.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <chrono>
#include "NsRender/VKFactory.h"
#include "Utils/Message.hpp"

namespace lve {

    App::App() {

        window.setFullscreenToggleCallback([this]() {
            requestSwapchainRecreate = true;
        });
        loadModels();
        createDescriptorSetLayout();
        createLoadingDescriptorPool();
        createLoadingDescriptorSet();
        texture = std::make_unique<Texture>(device, "resources/textures/logo/Kingscraft-Logo.png");
        updateLoadingDescriptorSet();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();

        uiSystem = std::make_unique<UiSystem>();

        // 1. Hook up Mouse Movement
        window.setMouseMoveCallback([this](double x, double y) {
            uiSystem->onMouseMove(x, y);
        });

        // 2. Hook up Mouse Buttons
        // Inside FirstApp constructor
        window.setMouseButtonCallback([this](int button, int action, int mods) {
            // Option A: Use the last saved position (Now works because of Step 1).
            double x = window.getLastX();
            double y = window.getLastY();

            uiSystem->onMouseButton(button, action, mods, x, y);
        });

        uiSystem->setQuitCallback([this]() {
            glfwSetWindowShouldClose(window.getGLFWWindow(), GLFW_TRUE);
        });

        QueueFamilyIndices indices = device.findPhysicalQueueFamilies();

        NoesisApp::VKFactory::InstanceInfo info{};
        info.instance = device.getInstance();
        info.physicalDevice = device.getPhysicalDevice();
        info.device = device.device();
        info.pipelineCache = VK_NULL_HANDLE;
        info.queueFamilyIndex = indices.graphicsFamily;
        info.vkGetInstanceProcAddr = glfwGetInstanceProcAddress;
        info.stereoSupport = false;

        uiSystem->init(window.getExtent().width, window.getExtent().height, info, swapchain->getRenderPass());
    }

    App::~App() {
        vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayout, nullptr);
        vkDestroyDescriptorPool(device.device(), loadingDescriptorPool, nullptr);
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    }

    void App::run() {

    // 🔥 Main loop
    while (!window.shouldClose()) {
        glfwPollEvents();
        window.processInput();

        auto currentExtent = window.getExtent();

        if ((requestSwapchainRecreate || window.wasWindowResized()) &&
            currentExtent.width > 0 && currentExtent.height > 0) {

            window.resetWindowResizedFlag();
            recreateSwapChain();

            lastExtent = currentExtent;
            uiSystem->resize(currentExtent.width, currentExtent.height);
            requestSwapchainRecreate = false;
        }

        drawFrame();
    }

    vkDeviceWaitIdle(device.device());
}



    void App::loadModels() {
        // std::vector<LveModel::Vertex> vertices = {
        //         // First triangle
        //         {{-1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // bottom-left
        //         {{1.0f, -1.0f},  {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // bottom-right
        //         {{1.0f, 1.0f},   {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // top-right
        //
        //         // Second triangle
        //         {{-1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // bottom-left
        //         {{1.0f, 1.0f},   {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // top-right
        //         {{-1.0f, 1.0f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // top-left
        // };
        //
        // lveModel = std::make_unique<LveModel>(lveDevice, vertices);
    }


    void App::createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 0;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &samplerLayoutBinding;

        if (vkCreateDescriptorSetLayout(device.device(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

    }

    void App::createLoadingDescriptorPool() {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 1;

        if (vkCreateDescriptorPool(device.device(), &poolInfo, nullptr, &loadingDescriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create loading descriptor pool!");
        }
    }

    void App::createLoadingDescriptorSet() {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = loadingDescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &descriptorSetLayout;  // or a separate layout if you want

        if (vkAllocateDescriptorSets(device.device(), &allocInfo, &loadingDescriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate loading descriptor set!");
        }
    }

    void App::updateLoadingDescriptorSet() {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = getTextureImageView();
        imageInfo.sampler = getTextureSampler();

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = loadingDescriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device.device(), 1, &descriptorWrite, 0, nullptr);
    }


    void App::createPipelineLayout() {

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(glm::vec2); // screenSize

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }


    void App::createPipeline() {
    assert(swapchain != nullptr && "Cannot create pipeline before swap chain");
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::dafaultPipelineConfigInfo(pipelineConfig);

    // 🔹 Vertex input (still needed for UI quads / meshes)
    auto bindingDescriptions = Model::Vertex::getBindingDescriptions();
    auto attributeDescriptions = Model::Vertex::getAttributeDescriptions();

    pipelineConfig.bindingDescriptions = bindingDescriptions;
    pipelineConfig.attributeDescriptions = attributeDescriptions;

    // 🔹 UI RenderPass (NO depth version of swapchain)
    pipelineConfig.renderPass = swapchain->getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;

    // =========================================================
    // 🎨 UI FIXES START HERE
    // =========================================================

    // ❌ Disable depth testing (UI does not need it)
    pipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
    pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
    pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS;
    pipelineConfig.depthStencilInfo.stencilTestEnable = VK_FALSE;

    // ✔ Enable alpha blending for UI transparency
    pipelineConfig.blendAttachmentState.blendEnable = VK_TRUE;

    pipelineConfig.blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    pipelineConfig.blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    pipelineConfig.blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;

    pipelineConfig.blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    pipelineConfig.blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    pipelineConfig.blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    // =========================================================
    // 🎨 UI FIXES END
    // =========================================================

    pipeline = std::make_unique<Pipeline>(
        device,
        "resources/shaders/simple_shader.vert.spv",
        "resources/shaders/simple_shader.frag.spv",
        pipelineConfig
    );
}

    void App::recreateSwapChain() {
        auto extent = window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = window.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device.device());

        // 1. Existing Swapchain Logic
        if (swapchain == nullptr) {
            swapchain = std::make_unique<SwapChain>(device, extent);
        } else {
            swapchain = std::make_unique<SwapChain>(device, extent, std::move(swapchain));
            if (swapchain->imageCount() != commandBuffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }
        createPipeline();
    }


    void App::createCommandBuffers() {
        commandBuffers.resize(swapchain->imageCount());
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void App::freeCommandBuffers() {
        vkFreeCommandBuffers(device.device(), device.getCommandPool(),
                             static_cast<uint32_t> (commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    void App::recordCommandBuffer(int imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        uiSystem->renderOffscreen(commandBuffers[imageIndex]);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = swapchain->getRenderPass();
        renderPassInfo.framebuffer = swapchain->getFrameBuffer(imageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapchain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float> (swapchain->getSwapChainExtent().width);
        viewport.height = static_cast<float> (swapchain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, swapchain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

        pipeline->bind(commandBuffers[imageIndex]);

        glm::vec2 screenSize = {
            static_cast<float>(swapchain->getSwapChainExtent().width),
            static_cast<float>(swapchain->getSwapChainExtent().height)
        };

        vkCmdPushConstants(commandBuffers[imageIndex], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::vec2), &screenSize);
        // lveModel->bind(commandBuffers[imageIndex]);.
        // lveModel->draw(commandBuffers[imageIndex]);

        uiSystem->render(commandBuffers[imageIndex]);

        vkCmdEndRenderPass(commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void App::drawFrame() {
        uint32_t imageIndex;
        auto result = swapchain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        uiSystem->update(glfwGetTime());
        recordCommandBuffer(imageIndex);
        result = swapchain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) {
            window.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to submit command buffers!");
        }
    }

}  // namespace lve