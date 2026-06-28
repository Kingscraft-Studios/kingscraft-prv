#include "Core/World/WorldScreen.hpp"
#include "Vulkan/App.hpp"
#include "Vulkan/TextureCache.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/RendererSettings.hpp"
#include "Resource/ResourceManager.hpp"
#include "Core/KeyBindHandler.hpp"
#include "Core/Registries.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
#include <cassert>
#include <thread>
#include <glm/gtc/matrix_transform.hpp>

namespace lve {

    WorldScreen::WorldScreen(VkExtent2D extent)
        : extent_(extent) {}

    WorldScreen::~WorldScreen() {
        cleanup();
    }

    void WorldScreen::init() {
        auto& device = App::get().getDevice();
        auto& resourceManager = App::get().getResourceManager();
        auto& keybinds = App::get().getKeyBindHandler();
        auto& window = App::get().getWindow();
        auto& textureCache = App::get().getTextureCache();

        // Wait for block registrations to complete
        while (!Registries::isBuilt()) {
            std::this_thread::yield();
        }

        textureCache.updateFromRegistry();

        createPipelineLayout();

        // Create world (chunks will load on first tick)
        world_ = std::make_unique<World>(device, 16, 24);

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
        camera_.setPosition({67.5f, 15.0f, 67.5f});
        camera_.setRotation(0.0f, -35.0f);

        glfwSetInputMode(window.getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        keybinds.setLayerEnabled(BindLayer::UI, false);
        fpsCounter_.init(App::get().getUiSystem());
        App::get().getUiSystem().resize(static_cast<int>(extent_.width),
                                        static_cast<int>(extent_.height));
        cursorCaptured_ = true;
        lastMouseX_ = window.getLastX();
        lastMouseY_ = window.getLastY();
    }

    void WorldScreen::tick(double dt) {
        float speed = 3.0f * static_cast<float>(dt);
        auto& keybinds = App::get().getKeyBindHandler();
        auto& window = App::get().getWindow();

        if (keybinds.isDown(Keys::W)) camera_.moveForward(speed);
        if (keybinds.isDown(Keys::S)) camera_.moveForward(-speed);
        if (keybinds.isDown(Keys::A)) camera_.moveRight(-speed);
        if (keybinds.isDown(Keys::D)) camera_.moveRight(speed);
        if (keybinds.isDown(Keys::SPACE)) camera_.moveUp(speed);
        if (keybinds.isDown(Keys::LEFT_SHIFT)) camera_.moveUp(-speed);

        double mx = window.getLastX();
        double my = window.getLastY();
        double dx = mx - lastMouseX_;
        double dy = lastMouseY_ - my;
        lastMouseX_ = mx;
        lastMouseY_ = my;

        if (dx != 0.0 || dy != 0.0) {
            camera_.rotate(static_cast<float>(dx) * 0.1f, static_cast<float>(dy) * 0.1f);
        }

        world_->update(camera_.getPosition().x, camera_.getPosition().z, RendererSettings::get().renderDistance);
    }

    void WorldScreen::render(const FrameContext& ctx) {
        fpsCounter_.setCpuGpuTimes(App::get().getCpuFrameTimeMs(),
                                   App::get().getRenderer().getGpuFrameTimeMs());
        fpsCounter_.update();
        world_->flushPendingCleanup();
        world_->processCompletedChunks();
        if (!pipeline_) return;

        pipeline_->bind(ctx.cmd);

        auto& settings = RendererSettings::get();
        float aspect = static_cast<float>(ctx.extent.width) / static_cast<float>(ctx.extent.height);
        camera_.setAspectRatio(aspect);
        camera_.setFov(settings.fov);
        camera_.setNearPlane(settings.nearPlane);
        camera_.setFarPlane(settings.farPlane);
        glm::mat4 viewProj = camera_.getProjectionMatrix() * camera_.getViewMatrix();

        vkCmdPushConstants(ctx.cmd, pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &viewProj);

        // Bind texture descriptor set
        auto& textureCache = App::get().getTextureCache();
        VkDescriptorSet texSet = textureCache.getDescriptorSet();
        if (texSet != VK_NULL_HANDLE) {
            vkCmdBindDescriptorSets(ctx.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout_, 0, 1, &texSet, 0, nullptr);
        }

        float chunkSize = static_cast<float>(world_->getChunkSize() - 1);
        float worldHeight = static_cast<float>(world_->getHeight());

        if (settings.enableFrustumCulling) {
            struct FrustumPlane { glm::vec3 normal; float d; };
            auto extractPlanes = [](const glm::mat4& m) -> std::array<FrustumPlane, 6> {
                std::array<FrustumPlane, 6> p;
                p[0] = {glm::vec3(m[0][3] + m[0][0], m[1][3] + m[1][0], m[2][3] + m[2][0]), m[3][3] + m[3][0]};
                p[1] = {glm::vec3(m[0][3] - m[0][0], m[1][3] - m[1][0], m[2][3] - m[2][0]), m[3][3] - m[3][0]};
                p[2] = {glm::vec3(m[0][3] + m[0][1], m[1][3] + m[1][1], m[2][3] + m[2][1]), m[3][3] + m[3][1]};
                p[3] = {glm::vec3(m[0][3] - m[0][1], m[1][3] - m[1][1], m[2][3] - m[2][1]), m[3][3] - m[3][1]};
                p[4] = {glm::vec3(m[0][3] + m[0][2], m[1][3] + m[1][2], m[2][3] + m[2][2]), m[3][3] + m[3][2]};
                p[5] = {glm::vec3(m[0][3] - m[0][2], m[1][3] - m[1][2], m[2][3] - m[2][2]), m[3][3] - m[3][2]};
                for (auto& pl : p) {
                    float len = glm::length(pl.normal);
                    pl.normal /= len;
                    pl.d /= len;
                }
                return p;
            };

            auto frustum = extractPlanes(viewProj);
            for (Chunk* chunk : world_->getLoadedChunks()) {
                glm::vec3 origin = chunk->getWorldOrigin();
                glm::vec3 min = origin;
                glm::vec3 max = origin + glm::vec3(chunkSize, worldHeight, chunkSize);

                bool visible = true;
                for (const auto& plane : frustum) {
                    glm::vec3 pv{
                        plane.normal.x >= 0 ? max.x : min.x,
                        plane.normal.y >= 0 ? max.y : min.y,
                        plane.normal.z >= 0 ? max.z : min.z,
                    };
                    if (glm::dot(plane.normal, pv) + plane.d < 0) {
                        visible = false;
                        break;
                    }
                }
                if (!visible) continue;

                chunk->bindAndDraw(ctx.cmd);
            }
        } else {
            for (Chunk* chunk : world_->getLoadedChunks()) {
                chunk->bindAndDraw(ctx.cmd);
            }
        }

        App::get().getUiSystem().render(ctx.cmd, ctx.renderPass);
    }

    void WorldScreen::renderGlow(const FrameContext& ctx) {
        (void)ctx;
    }

    void WorldScreen::cleanup() {
        fpsCounter_.cleanup(App::get().getUiSystem());
        if (cursorCaptured_) {
            glfwSetInputMode(App::get().getWindow().getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            cursorCaptured_ = false;
        }
        world_.reset();
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
            true
        };
    }

    void WorldScreen::createPipelineLayout() {
        auto& textureCache = App::get().getTextureCache();

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(glm::mat4);

        VkDescriptorSetLayout texLayout = textureCache.getLayout();

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &pushConstantRange;
        layoutInfo.setLayoutCount = (texLayout != VK_NULL_HANDLE) ? 1 : 0;
        layoutInfo.pSetLayouts = (texLayout != VK_NULL_HANDLE) ? &texLayout : nullptr;

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
