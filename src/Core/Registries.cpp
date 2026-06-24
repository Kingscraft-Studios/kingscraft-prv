#include "Core/Registries.hpp"

#include "Core/Blocks/Blocks.hpp"

namespace lve {
    std::atomic<bool> Registries::built_{false};

    void Registries::build() {
        Blocks::registerBlocks();
        built_.store(true);
    }
}
