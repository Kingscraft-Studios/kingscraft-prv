#pragma once
#include "Block.hpp"

namespace lve {
    class AirBlock : public Block {
    public:
        bool isSolid() const override { return false; }
        bool isOpaque() const override { return false; }
        bool isReplaceable() const override { return true; }
        int getLightAbsorption() const override { return 0; }
        float getHardness() const override { return 0.0f; }
    };
}
