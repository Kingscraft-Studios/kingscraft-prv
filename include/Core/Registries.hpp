#pragma once

#include <atomic>

namespace lve {
    class Registries {
    public:
        static void build();
        static bool isBuilt() { return built_.load(); }
    private:
        static std::atomic<bool> built_;
    };
}
