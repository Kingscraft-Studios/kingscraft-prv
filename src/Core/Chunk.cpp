#include "Core/Chunk.hpp"

namespace lve {

    Chunk::Chunk(Device& device, glm::ivec2 gridPos, int verticesPerAxis, float spacing)
        : device_(device)
        , gridPos_(gridPos)
        , verticesPerAxis_(verticesPerAxis)
        , spacing_(spacing)
    {
        float size = static_cast<float>(verticesPerAxis - 1) * spacing;
        worldOrigin_ = glm::vec3(
            static_cast<float>(gridPos.x) * size,
            0.0f,
            static_cast<float>(gridPos.y) * size
        );
    }

    void Chunk::upload() {
        VkDeviceSize vertexSize = vertices_.size() * sizeof(ChunkVertex);
        if (vertexSize > 0) {
            vertexBuffer_ = std::make_unique<Buffer>(
                device_,
                vertexSize,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
            vertexBuffer_->write(vertices_.data(), 0, vertexSize);
        }

        VkDeviceSize indexSize = indices_.size() * sizeof(uint16_t);
        if (indexSize > 0) {
            indexBuffer_ = std::make_unique<Buffer>(
                device_,
                indexSize,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
            indexBuffer_->write(indices_.data(), 0, indexSize);
            indexCount_ = static_cast<uint32_t>(indices_.size());
        }

        vertices_.clear();
        vertices_.shrink_to_fit();
        indices_.clear();
        indices_.shrink_to_fit();
    }

    void Chunk::bindAndDraw(VkCommandBuffer cmd) const {
        if (!vertexBuffer_ || !indexBuffer_) return;

        VkBuffer vb[] = {vertexBuffer_->getHandle()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, vb, offsets);
        vkCmdBindIndexBuffer(cmd, indexBuffer_->getHandle(), 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(cmd, indexCount_, 1, 0, 0, 0);
    }

    void Chunk::cleanup() {
        vertexBuffer_.reset();
        indexBuffer_.reset();
    }

} // namespace lve
