#pragma once

#include "Core/Registry.hpp"
#include "Core/Key.hpp"
#include "Block.hpp"

namespace lve {

class Blocks {
public:
    inline static const RegistryKey<Block> AIR = Registry<Block>::getRegistry().createKey();
    inline static const RegistryKey<Block> GRASS_BLOCK = Registry<Block>::getRegistry().createKey();

    static void registerBlocks();
};

} // namespace lve
