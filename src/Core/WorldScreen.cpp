#include "Core/WorldScreen.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>

namespace lve {

    WorldScreen::WorldScreen(Device& device, ResourceManager& resourceManager,
                             KeyBindHandler& keybinds, Window& window,
                             VkExtent2D extent, const std::vector<VkImageView>& swapChainImageViews,
                             VkFormat swapChainImageFormat)
        : device_(device)
        , resourceManager_(resourceManager)
        , keybinds_(keybinds)
        , window_(window)
        , extent_(extent)
        , swapChainImageViews_(swapChainImageViews)
        , swapChainImageFormat_(swapChainImageFormat) {}

    WorldScreen::~WorldScreen() {
        cleanup();
    }

    VkFormat WorldScreen::findDepthFormat() {
        return device_.findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    void WorldScreen::init() {
        worldRenderPass_ = RenderPass::createWithDepth(device_, swapChainImageFormat_, findDepthFormat());
        createDepthResources(extent_);
        createFramebuffers(swapChainImageViews_, extent_);

        createPipelineLayout();

        chunks_.reserve(9 * 9);
        for (int gz = 0; gz < 9; ++gz) {
            for (int gx = 0; gx < 9; ++gx) {
                Chunk chunk(device_, {gx, gz}, 16, 1.0f);
                ChunkMesher::generate(chunk, terrainNoise_);
                chunk.upload();
                chunks_.push_back(std::move(chunk));
            }
        }

        auto shadersReady = std::make_shared<int>(0);
        resourceManager_.loadShader("resources/shaders/terrain.vert.spv",
            [this, shadersReady](const std::vector<char>& data) {
                vertShaderCode_ = data;
                if (++(*shadersReady) == 2)
                    createPipeline();
            });
        resourceManager_.loadShader("resources/shaders/terrain.frag.spv",
            [this, shadersReady](const std::vector<char>& data) {
                fragShaderCode_ = data;
                if (++(*shadersReady) == 2)
                    createPipeline();
            });

        float aspect = static_cast<float>(extent_.width) / static_cast<float>(extent_.height);
        camera_.setAspectRatio(aspect);
        camera_.setPosition({67.5f, 30.0f, 67.5f});
        camera_.setRotation(0.0f, -45.0f);

        glfwSetInputMode(window_.getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        keybinds_.setNoesisInputEnabled(false);
        cursorCaptured_ = true;
        lastMouseX_ = window_.getLastX();
        lastMouseY_ = window_.getLastY();
    }

    void WorldScreen::tick(double dt) {
        if (!cursorCaptured_) return;

        float speed = 3.0f * static_cast<float>(dt);

        if (keybinds_.isDown(Key::W)) camera_.moveForward(speed);
        if (keybinds_.isDown(Key::S)) camera_.moveForward(-speed);
        if (keybinds_.isDown(Key::A)) camera_.moveRight(-speed);
        if (keybinds_.isDown(Key::D)) camera_.moveRight(speed);
        if (keybinds_.isDown(Key::SPACE)) camera_.moveUp(speed);
        if (keybinds_.isDown(Key::LEFT_SHIFT)) camera_.moveUp(-speed);

        double mx = window_.getLastX();
        double my = window_.getLastY();
        double dx = mx - lastMouseX_;
        double dy = lastMouseY_ - my;
        lastMouseX_ = mx;
        lastMouseY_ = my;

        if (dx != 0.0 || dy != 0.0) {
            camera_.rotate(static_cast<float>(dx) * 0.1f, static_cast<float>(dy) * 0.1f);
        }
    }

    void WorldScreen::render(const FrameContext& ctx) {
        if (!pipeline_) return;

        pipeline_->bind(ctx.cmd);

        float aspect = static_cast<float>(ctx.extent.width) / static_cast<float>(ctx.extent.height);
        camera_.setAspectRatio(aspect);
        glm::mat4 viewProj = camera_.getProjectionMatrix() * camera_.getViewMatrix();

        vkCmdPushConstants(ctx.cmd, pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &viewProj);

        for (auto& chunk : chunks_) {
            chunk.bindAndDraw(ctx.cmd);
        }
    }

    void WorldScreen::cleanup() {
        if (cursorCaptured_) {
            glfwSetInputMode(window_.getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            cursorCaptured_ = false;
        }
        for (auto& chunk : chunks_) chunk.cleanup();
        chunks_.clear();
        pipeline_.reset();
        if (pipelineLayout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_.device(), pipelineLayout_, nullptr);
            pipelineLayout_ = VK_NULL_HANDLE;
        }
        destroyFramebuffers();
        destroyDepthResources();
        worldRenderPass_.reset();
        swapChainImageViews_.clear();
    }

    void WorldScreen::onRenderPassChanged(VkRenderPass renderPass) {
        (void)renderPass;
    }

    void WorldScreen::onSwapChainRecreated(VkExtent2D extent, const std::vector<VkImageView>& swapChainImageViews, VkFormat swapChainImageFormat) {
        destroyFramebuffers();
        destroyDepthResources();

        extent_ = extent;
        swapChainImageViews_ = swapChainImageViews;

        if (swapChainImageFormat != swapChainImageFormat_) {
            worldRenderPass_ = RenderPass::createWithDepth(device_, swapChainImageFormat, findDepthFormat());
            swapChainImageFormat_ = swapChainImageFormat;
        }

        createDepthResources(extent_);
        createFramebuffers(swapChainImageViews_, extent_);
    }

    void WorldScreen::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(glm::mat4);

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &pushConstantRange;
        layoutInfo.setLayoutCount = 0;
        layoutInfo.pSetLayouts = nullptr;

        if (vkCreatePipelineLayout(device_.device(), &layoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void WorldScreen::createPipeline() {
        assert(!vertShaderCode_.empty() && !fragShaderCode_.empty());

        PipelineConfigInfo configInfo{};
        Pipeline::defaultPipelineConfigInfo(configInfo);

        auto bindingDesc = ChunkVertex::getBindingDescription();
        auto attributeDescs = ChunkVertex::getAttributeDescriptions();
        configInfo.bindingDescriptions = {bindingDesc};
        configInfo.attributeDescriptions = {attributeDescs.begin(), attributeDescs.end()};

        configInfo.renderPass = worldRenderPass_->getHandle();
        configInfo.pipelineLayout = pipelineLayout_;

        pipeline_ = std::make_unique<Pipeline>(device_, vertShaderCode_, fragShaderCode_, configInfo);
    }

    void WorldScreen::createDepthResources(VkExtent2D extent) {
        VkFormat depthFormat = findDepthFormat();
        size_t imageCount = swapChainImageViews_.size();

        depthImages_.resize(imageCount);
        depthImageMemories_.resize(imageCount);
        depthImageViews_.resize(imageCount);

        for (size_t i = 0; i < imageCount; i++) {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = extent.width;
            imageInfo.extent.height = extent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            if (vkCreateImage(device_.device(), &imageInfo, nullptr, &depthImages_[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create depth image!");
            }

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(device_.device(), depthImages_[i], &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = device_.findMemoryType(
                memRequirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );

            if (vkAllocateMemory(device_.device(), &allocInfo, nullptr, &depthImageMemories_[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate depth image memory!");
            }

            vkBindImageMemory(device_.device(), depthImages_[i], depthImageMemories_[i], 0);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = depthImages_[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device_.device(), &viewInfo, nullptr, &depthImageViews_[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create depth image view!");
            }
        }
    }

    void WorldScreen::createFramebuffers(const std::vector<VkImageView>& swapChainImageViews, VkExtent2D extent) {
        size_t imageCount = depthImages_.size();
        framebuffers_.resize(imageCount);

        for (size_t i = 0; i < imageCount; i++) {
            std::array<VkImageView, 2> attachments = {
                swapChainImageViews[i],
                depthImageViews_[i]
            };

            VkFramebufferCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            info.renderPass = worldRenderPass_->getHandle();
            info.attachmentCount = static_cast<uint32_t>(attachments.size());
            info.pAttachments = attachments.data();
            info.width = extent.width;
            info.height = extent.height;
            info.layers = 1;

            if (vkCreateFramebuffer(device_.device(), &info, nullptr, &framebuffers_[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create world framebuffer!");
            }
        }
    }

    void WorldScreen::destroyDepthResources() {
        for (size_t i = 0; i < depthImageViews_.size(); i++) {
            vkDestroyImageView(device_.device(), depthImageViews_[i], nullptr);
            vkDestroyImage(device_.device(), depthImages_[i], nullptr);
            vkFreeMemory(device_.device(), depthImageMemories_[i], nullptr);
        }
        depthImageViews_.clear();
        depthImages_.clear();
        depthImageMemories_.clear();
    }

    void WorldScreen::destroyFramebuffers() {
        for (auto fb : framebuffers_) {
            vkDestroyFramebuffer(device_.device(), fb, nullptr);
        }
        framebuffers_.clear();
    }

} // namespace lve
