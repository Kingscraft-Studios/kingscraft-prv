#pragma once
#include "Core/Blocks/Block.hpp"

namespace lve {
    class GrassBlock : public Block {
    public:
        using Block::Block;
        float getHardness() const override { return 0.6f; }
    };
}
