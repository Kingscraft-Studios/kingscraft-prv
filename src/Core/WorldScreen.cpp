#include "Core/WorldScreen.hpp"
#include "Vulkan/App.hpp"
#include "Renderer/Renderer.hpp"
#include "Resource/ResourceManager.hpp"
#include "Core/KeyBindHandler.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>

namespace lve {

    WorldScreen::WorldScreen(VkExtent2D extent)
        : extent_(extent) {}

    WorldScreen::~WorldScreen() {
        cleanup();
    }

    void WorldScreen::init() {
        createPipelineLayout();

        auto& device = App::get().getDevice();
        auto& resourceManager = App::get().getResourceManager();
        auto& keybinds = App::get().getKeyBindHandler();
        auto& window = App::get().getWindow();

        chunks_.reserve(9 * 9);
        for (int gz = 0; gz < 9; ++gz) {
            for (int gx = 0; gx < 9; ++gx) {
                Chunk chunk(device, {gx, gz}, 16, 1.0f);
                ChunkMesher::generate(chunk, terrainNoise_);
                chunk.upload();
                chunks_.push_back(std::move(chunk));
            }
        }

        auto shadersReady = std::make_shared<int>(0);
        resourceManager.loadShader("resources/shaders/terrain.vert.spv",
            [this, shadersReady](const std::vector<char>& data) {
                vertShaderCode_ = data;
                if (++(*shadersReady) == 2)
                    createPipeline();
            });
        resourceManager.loadShader("resources/shaders/terrain.frag.spv",
            [this, shadersReady](const std::vector<char>& data) {
                fragShaderCode_ = data;
                if (++(*shadersReady) == 2)
                    createPipeline();
            });

        float aspect = static_cast<float>(extent_.width) / static_cast<float>(extent_.height);
        camera_.setAspectRatio(aspect);
        camera_.setPosition({67.5f, 30.0f, 67.5f});
        camera_.setRotation(0.0f, -45.0f);

        glfwSetInputMode(window.getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        keybinds.setNoesisInputEnabled(false);
        cursorCaptured_ = true;
        lastMouseX_ = window.getLastX();
        lastMouseY_ = window.getLastY();
    }

    void WorldScreen::tick(double dt) {
        if (!cursorCaptured_) return;

        float speed = 3.0f * static_cast<float>(dt);
        auto& keybinds = App::get().getKeyBindHandler();
        auto& window = App::get().getWindow();

        if (keybinds.isDown(Key::W)) camera_.moveForward(speed);
        if (keybinds.isDown(Key::S)) camera_.moveForward(-speed);
        if (keybinds.isDown(Key::A)) camera_.moveRight(-speed);
        if (keybinds.isDown(Key::D)) camera_.moveRight(speed);
        if (keybinds.isDown(Key::SPACE)) camera_.moveUp(speed);
        if (keybinds.isDown(Key::LEFT_SHIFT)) camera_.moveUp(-speed);

        double mx = window.getLastX();
        double my = window.getLastY();
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
            glfwSetInputMode(App::get().getWindow().getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            cursorCaptured_ = false;
        }
        for (auto& chunk : chunks_) chunk.cleanup();
        chunks_.clear();
        pipeline_.reset();
        if (pipelineLayout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(App::get().getDevice().device(), pipelineLayout_, nullptr);
            pipelineLayout_ = VK_NULL_HANDLE;
        }
    }

    void WorldScreen::onRenderPassChanged(VkRenderPass renderPass) {
        (void)renderPass;
    }

    void WorldScreen::onSwapChainRecreated(VkExtent2D extent) {
        extent_ = extent;
    }

    FrameRenderInfo WorldScreen::getFrameRenderInfo(const Renderer& renderer, uint32_t imageIndex) const {
        return {
            renderer.getWorldRenderPass(),
            renderer.getWorldFramebuffer(imageIndex),
            {{{0.4f, 0.6f, 0.9f, 1.0f}}, {1.0f, 0}},
            false
        };
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

        if (vkCreatePipelineLayout(App::get().getDevice().device(), &layoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void WorldScreen::createPipeline() {
        assert(!vertShaderCode_.empty() && !fragShaderCode_.empty());

        auto& device = App::get().getDevice();
        auto& renderer = App::get().getRenderer();

        PipelineConfigInfo configInfo{};
        Pipeline::defaultPipelineConfigInfo(configInfo);

        auto bindingDesc = ChunkVertex::getBindingDescription();
        auto attributeDescs = ChunkVertex::getAttributeDescriptions();
        configInfo.bindingDescriptions = {bindingDesc};
        configInfo.attributeDescriptions = {attributeDescs.begin(), attributeDescs.end()};

        configInfo.renderPass = renderer.getWorldRenderPass();
        configInfo.pipelineLayout = pipelineLayout_;

        pipeline_ = std::make_unique<Pipeline>(device, vertShaderCode_, fragShaderCode_, configInfo);
    }

} // namespace lve