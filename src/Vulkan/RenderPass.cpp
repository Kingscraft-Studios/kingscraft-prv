#include "Vulkan/RenderPass.hpp"
#include <array>
#include <stdexcept>

namespace lve {

    RenderPass::RenderPass(Device& device,
                           const std::vector<AttachmentDescription>& attachments,
                           const std::vector<VkSubpassDescription>& subpasses,
                           const std::vector<VkSubpassDependency>& dependencies)
        : device_(device) {

        std::vector<VkAttachmentDescription> vkAttachments;
        vkAttachments.reserve(attachments.size());
        for (const auto& a : attachments) {
            vkAttachments.push_back({
                .flags = 0,
                .format = a.format,
                .samples = a.samples,
                .loadOp = a.loadOp,
                .storeOp = a.storeOp,
                .stencilLoadOp = a.stencilLoadOp,
                .stencilStoreOp = a.stencilStoreOp,
                .initialLayout = a.initialLayout,
                .finalLayout = a.finalLayout
            });
        }

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(vkAttachments.size());
        renderPassInfo.pAttachments = vkAttachments.data();
        renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
        renderPassInfo.pSubpasses = subpasses.data();
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(device_.device(), &renderPassInfo, nullptr, &renderPass_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    RenderPass::~RenderPass() {
        vkDestroyRenderPass(device_.device(), renderPass_, nullptr);
    }

    std::unique_ptr<RenderPass> RenderPass::createDefault(Device& device, VkFormat colorFormat) {
        AttachmentDescription colorAttachment{};
        colorAttachment.format = colorFormat;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstSubpass = 0;
        dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask =
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        return std::make_unique<RenderPass>(
            device,
            std::vector{colorAttachment},
            std::vector{subpass},
            std::vector{dependency}
        );
    }

} // namespace lve
