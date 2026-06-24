#include "Renderer/Bloom.hpp"
#include <array>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace lve {

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file: " + filename);
    }
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

Bloom::Bloom(Device& device, VkExtent2D windowExtent, VkRenderPass sceneRenderPass,
             DescriptorManager& descriptorManager)
    : device_(device), descriptorManager_(descriptorManager),
      sceneRenderPass_(sceneRenderPass), windowExtent_(windowExtent) {
    createOffscreen();
    createUniformBuffers();
    createDescriptors();
    createPipelines();
}

Bloom::~Bloom() {
    vkDeviceWaitIdle(device_.device());

    for (auto& pipe : {pipelines_.blurVert, pipelines_.blurHorz, pipelines_.glowPass}) {
        if (pipe != VK_NULL_HANDLE)
            vkDestroyPipeline(device_.device(), pipe, nullptr);
    }
    for (auto& layout : {pipelineLayouts_.blur, pipelineLayouts_.scene}) {
        if (layout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(device_.device(), layout, nullptr);
    }
    for (auto& layout : {descriptorSetLayouts_.blur, descriptorSetLayouts_.scene}) {
        if (layout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(device_.device(), layout, nullptr);
    }
    if (descriptorPool_ != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device_.device(), descriptorPool_, nullptr);

    for (auto& frame : frames_) {
        frame.blurUBO.reset();
        frame.glowUBO.reset();
    }

    if (offscreenPass_.sampler != VK_NULL_HANDLE)
        vkDestroySampler(device_.device(), offscreenPass_.sampler, nullptr);
    destroyOffscreenFramebuffers();
    if (offscreenPass_.renderPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(device_.device(), offscreenPass_.renderPass, nullptr);
}

void Bloom::computeOffscreenDim(VkExtent2D windowExtent, int32_t& outW, int32_t& outH) {
    float aspect = static_cast<float>(windowExtent.width) / static_cast<float>(windowExtent.height);
    if (aspect >= 1.0f) {
        outW = static_cast<int32_t>(static_cast<float>(MAX_FB_DIM) * aspect + 0.5f);
        outH = MAX_FB_DIM;
    } else {
        outW = MAX_FB_DIM;
        outH = static_cast<int32_t>(static_cast<float>(MAX_FB_DIM) / aspect + 0.5f);
    }
}

void Bloom::destroyFramebuffer(FrameBuffer& fb) {
    if (fb.framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(device_.device(), fb.framebuffer, nullptr);
        fb.framebuffer = VK_NULL_HANDLE;
    }
    if (fb.color.view) { vkDestroyImageView(device_.device(), fb.color.view, nullptr); fb.color.view = VK_NULL_HANDLE; }
    if (fb.color.image) { vkDestroyImage(device_.device(), fb.color.image, nullptr); fb.color.image = VK_NULL_HANDLE; }
    if (fb.color.mem) { vkFreeMemory(device_.device(), fb.color.mem, nullptr); fb.color.mem = VK_NULL_HANDLE; }
    if (fb.depth.view) { vkDestroyImageView(device_.device(), fb.depth.view, nullptr); fb.depth.view = VK_NULL_HANDLE; }
    if (fb.depth.image) { vkDestroyImage(device_.device(), fb.depth.image, nullptr); fb.depth.image = VK_NULL_HANDLE; }
    if (fb.depth.mem) { vkFreeMemory(device_.device(), fb.depth.mem, nullptr); fb.depth.mem = VK_NULL_HANDLE; }
}

void Bloom::destroyOffscreenFramebuffers() {
    for (auto& fb : offscreenPass_.framebuffers)
        destroyFramebuffer(fb);
}

void Bloom::recreate(VkExtent2D windowExtent, VkRenderPass sceneRenderPass) {
    bool dimsChanged = windowExtent.width != windowExtent_.width || windowExtent.height != windowExtent_.height;
    windowExtent_ = windowExtent;

    if (dimsChanged) {
        destroyOffscreenFramebuffers();
        computeOffscreenDim(windowExtent_, offscreenPass_.width, offscreenPass_.height);
        createOffscreenFramebuffer(&offscreenPass_.framebuffers[0], FB_COLOR_FORMAT, depthFormat_);
        createOffscreenFramebuffer(&offscreenPass_.framebuffers[1], FB_COLOR_FORMAT, depthFormat_);
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            updateFrameDescriptor(i);
    }

    if (sceneRenderPass_ != sceneRenderPass) {
        sceneRenderPass_ = sceneRenderPass;
        if (pipelines_.blurHorz != VK_NULL_HANDLE) {
            vkDestroyPipeline(device_.device(), pipelines_.blurHorz, nullptr);
            pipelines_.blurHorz = VK_NULL_HANDLE;
        }
        createBlurPipeline(sceneRenderPass_, 1, pipelines_.blurHorz);
    }
}

void Bloom::createOffscreenFramebuffer(FrameBuffer* fb, VkFormat colorFormat, VkFormat depthFormat) {
    int32_t w = offscreenPass_.width;
    int32_t h = offscreenPass_.height;

    VkImageCreateInfo image{};
    image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = colorFormat;
    image.extent.width = static_cast<uint32_t>(w);
    image.extent.height = static_cast<uint32_t>(h);
    image.extent.depth = 1;
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    device_.createImageWithInfo(image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, fb->color.image, fb->color.mem);

    VkImageViewCreateInfo view{};
    view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = colorFormat;
    view.image = fb->color.image;
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.levelCount = 1;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device_.device(), &view, nullptr, &fb->color.view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create offscreen color image view!");
    }

    image.format = depthFormat;
    image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    device_.createImageWithInfo(image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, fb->depth.image, fb->depth.mem);

    view.format = depthFormat;
    view.image = fb->depth.image;
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (vkCreateImageView(device_.device(), &view, nullptr, &fb->depth.view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create offscreen depth image view!");
    }

    std::array<VkImageView, 2> attachments = {fb->color.view, fb->depth.view};

    VkFramebufferCreateInfo fbInfo{};
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.renderPass = offscreenPass_.renderPass;
    fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    fbInfo.pAttachments = attachments.data();
    fbInfo.width = static_cast<uint32_t>(w);
    fbInfo.height = static_cast<uint32_t>(h);
    fbInfo.layers = 1;

    if (vkCreateFramebuffer(device_.device(), &fbInfo, nullptr, &fb->framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create offscreen framebuffer!");
    }

    fb->descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    fb->descriptor.imageView = fb->color.view;
    fb->descriptor.sampler = offscreenPass_.sampler;
}

void Bloom::createOffscreen() {
    computeOffscreenDim(windowExtent_, offscreenPass_.width, offscreenPass_.height);

    VkFormat fbDepthFormat = device_.findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
    depthFormat_ = fbDepthFormat;

    std::array<VkAttachmentDescription, 2> attachments = {};

    attachments[0].format = FB_COLOR_FORMAT;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    attachments[1].format = fbDepthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef{};
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;

    std::array<VkSubpassDependency, 3> dependencies{};

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[2].srcSubpass = 0;
    dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[2].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo rpInfo{};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    rpInfo.pAttachments = attachments.data();
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &subpass;
    rpInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    rpInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(device_.device(), &rpInfo, nullptr, &offscreenPass_.renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create offscreen render pass!");
    }

    VkSamplerCreateInfo sampler{};
    sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler.magFilter = VK_FILTER_LINEAR;
    sampler.minFilter = VK_FILTER_LINEAR;
    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.mipLodBias = 0.0f;
    sampler.maxAnisotropy = 1.0f;
    sampler.minLod = 0.0f;
    sampler.maxLod = 1.0f;
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    if (vkCreateSampler(device_.device(), &sampler, nullptr, &offscreenPass_.sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create offscreen sampler!");
    }

    createOffscreenFramebuffer(&offscreenPass_.framebuffers[0], FB_COLOR_FORMAT, fbDepthFormat);
    createOffscreenFramebuffer(&offscreenPass_.framebuffers[1], FB_COLOR_FORMAT, fbDepthFormat);
}

void Bloom::createUniformBuffers() {
    for (auto& frame : frames_) {
        frame.glowUBO = std::make_unique<Buffer>(
            device_, sizeof(GlowUniformData),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        frame.blurUBO = std::make_unique<Buffer>(
            device_, sizeof(BlurUniformData),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }
}

void Bloom::createDescriptors() {
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT * 4},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT * 4}
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT * 4;

    if (vkCreateDescriptorPool(device_.device(), &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create bloom descriptor pool!");
    }

    std::vector<VkDescriptorSetLayoutBinding> bindings;

    bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
    };
    descriptorSetLayouts_.blur = descriptorManager_.createLayout(bindings);

    bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
    };
    descriptorSetLayouts_.scene = descriptorManager_.createLayout(bindings);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        frames_[i].blurVert = descriptorManager_.allocateSet(descriptorPool_, descriptorSetLayouts_.blur);
        frames_[i].blurHorz = descriptorManager_.allocateSet(descriptorPool_, descriptorSetLayouts_.blur);
        frames_[i].scene = descriptorManager_.allocateSet(descriptorPool_, descriptorSetLayouts_.scene);

        VkDescriptorBufferInfo blurBufferInfo{};
        blurBufferInfo.buffer = frames_[i].blurUBO->getHandle();
        blurBufferInfo.offset = 0;
        blurBufferInfo.range = sizeof(BlurUniformData);

        VkDescriptorBufferInfo glowBufferInfo{};
        glowBufferInfo.buffer = frames_[i].glowUBO->getHandle();
        glowBufferInfo.offset = 0;
        glowBufferInfo.range = sizeof(GlowUniformData);

        std::vector<VkWriteDescriptorSet> writes;

        writes = {
            {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
                frames_[i].blurVert, 0, 0, 1,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &blurBufferInfo, nullptr
            },
            {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
                frames_[i].blurVert, 1, 0, 1,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &offscreenPass_.framebuffers[0].descriptor, nullptr, nullptr
            }
        };
        descriptorManager_.updateDescriptorSets(writes);

        writes = {
            {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
                frames_[i].blurHorz, 0, 0, 1,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &blurBufferInfo, nullptr
            },
            {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
                frames_[i].blurHorz, 1, 0, 1,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &offscreenPass_.framebuffers[1].descriptor, nullptr, nullptr
            }
        };
        descriptorManager_.updateDescriptorSets(writes);

        writes = {
            {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
                frames_[i].scene, 0, 0, 1,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &glowBufferInfo, nullptr
            }
        };
        descriptorManager_.updateDescriptorSets(writes);
    }
}

void Bloom::updateFrameDescriptor(uint32_t i) {
    if (i >= MAX_FRAMES_IN_FLIGHT) return;

    VkDescriptorBufferInfo blurBufferInfo{};
    blurBufferInfo.buffer = frames_[i].blurUBO->getHandle();
    blurBufferInfo.offset = 0;
    blurBufferInfo.range = sizeof(BlurUniformData);

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

    // blurVert: sampler binding (binding 1) → fb[0]
    write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
             frames_[i].blurVert, 1, 0, 1,
             VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
             &offscreenPass_.framebuffers[0].descriptor, nullptr, nullptr};
    vkUpdateDescriptorSets(device_.device(), 1, &write, 0, nullptr);

    // blurHorz: sampler binding (binding 1) → fb[1]
    write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
             frames_[i].blurHorz, 1, 0, 1,
             VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
             &offscreenPass_.framebuffers[1].descriptor, nullptr, nullptr};
    vkUpdateDescriptorSets(device_.device(), 1, &write, 0, nullptr);
}

void Bloom::createBlurPipeline(VkRenderPass renderPass, uint32_t blurdirection, VkPipeline& outPipeline) {
    auto vertMod = [&]() {
        auto code = readFile("resources/shaders/PostProcess/bloom/gaussblur.vert.spv");
        VkShaderModuleCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        ci.codeSize = code.size();
        ci.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule mod;
        if (vkCreateShaderModule(device_.device(), &ci, nullptr, &mod) != VK_SUCCESS)
            throw std::runtime_error("failed to create gaussblur vert shader module");
        return mod;
    }();
    auto fragMod = [&]() {
        auto code = readFile("resources/shaders/PostProcess/bloom/gaussblur.frag.spv");
        VkShaderModuleCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        ci.codeSize = code.size();
        ci.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule mod;
        if (vkCreateShaderModule(device_.device(), &ci, nullptr, &mod) != VK_SUCCESS)
            throw std::runtime_error("failed to create gaussblur frag shader module");
        return mod;
    }();

    VkPipelineShaderStageCreateInfo shaderStages[2]{};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertMod;
    shaderStages[0].pName = "main";
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragMod;
    shaderStages[1].pName = "main";

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterization{};
    rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization.cullMode = VK_CULL_MODE_NONE;
    rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization.lineWidth = 1.0f;

    VkPipelineColorBlendAttachmentState blendAttachment{};
    blendAttachment.colorWriteMask = 0xF;
    blendAttachment.blendEnable = VK_TRUE;
    blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &blendAttachment;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineMultisampleStateCreateInfo multisample{};
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineVertexInputStateCreateInfo emptyInputState{};
    emptyInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkSpecializationMapEntry specEntry{};
    specEntry.constantID = 0;
    specEntry.offset = 0;
    specEntry.size = sizeof(uint32_t);

    VkSpecializationInfo specInfo{};
    specInfo.mapEntryCount = 1;
    specInfo.pMapEntries = &specEntry;
    specInfo.dataSize = sizeof(uint32_t);
    specInfo.pData = &blurdirection;

    shaderStages[1].pSpecializationInfo = &specInfo;

    VkGraphicsPipelineCreateInfo pipelineCI{};
    pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCI.stageCount = 2;
    pipelineCI.pStages = shaderStages;
    pipelineCI.pVertexInputState = &emptyInputState;
    pipelineCI.pInputAssemblyState = &inputAssembly;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pRasterizationState = &rasterization;
    pipelineCI.pMultisampleState = &multisample;
    pipelineCI.pColorBlendState = &colorBlending;
    pipelineCI.pDepthStencilState = &depthStencil;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.layout = pipelineLayouts_.blur;
    pipelineCI.renderPass = renderPass;

    if (vkCreateGraphicsPipelines(device_.device(), VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &outPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create blur pipeline!");
    }

    vkDestroyShaderModule(device_.device(), vertMod, nullptr);
    vkDestroyShaderModule(device_.device(), fragMod, nullptr);
}

void Bloom::createPipelines() {
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descriptorSetLayouts_.blur;
    layoutInfo.pushConstantRangeCount = 0;
    layoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device_.device(), &layoutInfo, nullptr, &pipelineLayouts_.blur) != VK_SUCCESS) {
        throw std::runtime_error("failed to create blur pipeline layout!");
    }

    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descriptorSetLayouts_.scene;

    if (vkCreatePipelineLayout(device_.device(), &layoutInfo, nullptr, &pipelineLayouts_.scene) != VK_SUCCESS) {
        throw std::runtime_error("failed to create scene pipeline layout!");
    }

    auto loadShaderModule = [this](const std::string& path) {
        auto code = readFile(path);
        VkShaderModuleCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        ci.codeSize = code.size();
        ci.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule mod;
        if (vkCreateShaderModule(device_.device(), &ci, nullptr, &mod) != VK_SUCCESS)
            throw std::runtime_error("failed to create shader module: " + path);
        return mod;
    };

    auto vertModColor = loadShaderModule("resources/shaders/PostProcess/bloom/colorpass.vert.spv");
    auto fragModColor = loadShaderModule("resources/shaders/PostProcess/bloom/colorpass.frag.spv");

    VkPipelineShaderStageCreateInfo shaderStages[2]{};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterization{};
    rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization.cullMode = VK_CULL_MODE_NONE;
    rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization.lineWidth = 1.0f;

    VkPipelineColorBlendAttachmentState blendAttachment{};
    blendAttachment.colorWriteMask = 0xF;
    blendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &blendAttachment;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineMultisampleStateCreateInfo multisample{};
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Create blur pipelines using the extracted method
    createBlurPipeline(offscreenPass_.renderPass, 0, pipelines_.blurVert);
    createBlurPipeline(sceneRenderPass_, 1, pipelines_.blurHorz);

    // --- Color pass (glow objects) ---
    shaderStages[0].module = vertModColor;
    shaderStages[0].pName = "main";
    shaderStages[1].module = fragModColor;
    shaderStages[1].pName = "main";
    shaderStages[1].pSpecializationInfo = nullptr;

    VkVertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(glm::vec3) * 2;
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> attribDescs{};
    attribDescs[0].binding = 0;
    attribDescs[0].location = 0;
    attribDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribDescs[0].offset = 0;
    attribDescs[1].binding = 0;
    attribDescs[1].location = 1;
    attribDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribDescs[1].offset = sizeof(glm::vec3);

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &bindingDesc;
    vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDescs.size());
    vertexInput.pVertexAttributeDescriptions = attribDescs.data();

    VkGraphicsPipelineCreateInfo pipelineCI{};
    pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCI.stageCount = 2;
    pipelineCI.pStages = shaderStages;
    pipelineCI.pVertexInputState = &vertexInput;
    pipelineCI.pInputAssemblyState = &inputAssembly;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pRasterizationState = &rasterization;
    pipelineCI.pMultisampleState = &multisample;
    pipelineCI.pColorBlendState = &colorBlending;
    pipelineCI.pDepthStencilState = &depthStencil;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.layout = pipelineLayouts_.scene;
    pipelineCI.renderPass = offscreenPass_.renderPass;

    rasterization.cullMode = VK_CULL_MODE_NONE;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;

    if (vkCreateGraphicsPipelines(device_.device(), VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipelines_.glowPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create glow pass pipeline!");
    }

    vkDestroyShaderModule(device_.device(), vertModColor, nullptr);
    vkDestroyShaderModule(device_.device(), fragModColor, nullptr);
}

void Bloom::preScene(VkCommandBuffer cmd, uint32_t frameIndex,
                     const std::function<void(const FrameContext&)>& renderGlow,
                     const FrameContext& ctx) {
    beginGlowPass(cmd, frameIndex);

    FrameContext glowCtx = ctx;
    glowCtx.cmd = cmd;
    glowCtx.frameIndex = frameIndex;
    renderGlow(glowCtx);

    endGlowPass(cmd, frameIndex);
}

void Bloom::postScene(VkCommandBuffer cmd, uint32_t frameIndex,
                      const FrameContext& ctx) {
    (void)ctx;
    compositeBloom(cmd, frameIndex);
}

void Bloom::beginGlowPass(VkCommandBuffer cmd, uint32_t frameIndex) {
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo rpBegin{};
    rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBegin.renderPass = offscreenPass_.renderPass;
    rpBegin.framebuffer = offscreenPass_.framebuffers[0].framebuffer;
    rpBegin.renderArea.extent.width = offscreenPass_.width;
    rpBegin.renderArea.extent.height = offscreenPass_.height;
    rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
    rpBegin.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.width = static_cast<float>(offscreenPass_.width);
    viewport.height = static_cast<float>(offscreenPass_.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent.width = offscreenPass_.width;
    scissor.extent.height = offscreenPass_.height;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayouts_.scene, 0, 1,
                            &frames_[frameIndex].scene, 0, nullptr);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines_.glowPass);
}

void Bloom::endGlowPass(VkCommandBuffer cmd, uint32_t frameIndex) {
    vkCmdEndRenderPass(cmd);

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo rpBegin{};
    rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBegin.renderPass = offscreenPass_.renderPass;
    rpBegin.framebuffer = offscreenPass_.framebuffers[1].framebuffer;
    rpBegin.renderArea.extent.width = offscreenPass_.width;
    rpBegin.renderArea.extent.height = offscreenPass_.height;
    rpBegin.clearValueCount = static_cast<uint32_t>(clearValues.size());
    rpBegin.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.width = static_cast<float>(offscreenPass_.width);
    viewport.height = static_cast<float>(offscreenPass_.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent.width = offscreenPass_.width;
    scissor.extent.height = offscreenPass_.height;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayouts_.blur, 0, 1,
                            &frames_[frameIndex].blurVert, 0, nullptr);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines_.blurVert);
    vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdEndRenderPass(cmd);
}

void Bloom::compositeBloom(VkCommandBuffer cmd, uint32_t frameIndex) {
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayouts_.blur, 0, 1,
                            &frames_[frameIndex].blurHorz, 0, nullptr);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines_.blurHorz);
    vkCmdDraw(cmd, 3, 1, 0, 0);
}

void Bloom::updateUniforms(uint32_t frameIndex, const GlowUniformData& glowData,
                           const BloomElement& element) {
    if (frameIndex >= MAX_FRAMES_IN_FLIGHT) return;

    frames_[frameIndex].glowUBO->write(&glowData, 0, sizeof(GlowUniformData));

    BlurUniformData blurData{};
    blurData.blurScale = element.radius;
    blurData.blurStrength = element.intensity;
    blurData.sigma = element.sigma;
    blurData.kernelSize = element.kernelSize;

    frames_[frameIndex].blurUBO->write(&blurData, 0, sizeof(BlurUniformData));
}

} // namespace lve
