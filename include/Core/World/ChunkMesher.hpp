#pragma once

#include "Core/World/Chunk.hpp"
#include <cstdint>
#include <vector>

namespace lve {

    class ChunkMesher {
    public:
        static void generate(Chunk& chunk, const std::vector<uint8_t>& blockIds, int height);
    };

} // namespace lve
