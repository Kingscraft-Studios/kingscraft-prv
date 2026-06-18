#pragma once

#include "Core/Chunk.hpp"
#include "Core/TerrainGenerator.hpp"

namespace lve {

    class ChunkMesher {
    public:
        static void generate(Chunk& chunk, const TerrainGenerator& noise);
    };

} // namespace lve
