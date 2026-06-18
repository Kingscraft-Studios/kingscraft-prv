#include "Core/ChunkMesher.hpp"

namespace lve {

    void ChunkMesher::generate(Chunk& chunk, const TerrainGenerator& noise) {
        int N = chunk.getVerticesPerAxis();
        glm::vec3 origin = chunk.getWorldOrigin();

        int vertexCount = N * N;
        int quadCount = (N - 1) * (N - 1);

        auto& vertices = chunk.vertices();
        auto& indices = chunk.indices();

        vertices.clear();
        vertices.reserve(vertexCount);
        indices.clear();
        indices.reserve(quadCount * 6);

        for (int z = 0; z < N; ++z) {
            for (int x = 0; x < N; ++x) {
                float wx = origin.x + static_cast<float>(x);
                float wz = origin.z + static_cast<float>(z);
                float h = noise.getHeight(wx, wz);
                vertices.push_back({{wx, h, wz}, TerrainGenerator::getColor(h)});
            }
        }

        for (int z = 0; z < N - 1; ++z) {
            for (int x = 0; x < N - 1; ++x) {
                int i = z * N + x;
                indices.push_back(static_cast<uint16_t>(i));
                indices.push_back(static_cast<uint16_t>(i + 1));
                indices.push_back(static_cast<uint16_t>(i + N));
                indices.push_back(static_cast<uint16_t>(i + 1));
                indices.push_back(static_cast<uint16_t>(i + N + 1));
                indices.push_back(static_cast<uint16_t>(i + N));
            }
        }
    }

} // namespace lve
