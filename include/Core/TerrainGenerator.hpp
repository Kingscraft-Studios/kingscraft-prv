#pragma once

#include "FastNoiseLite.h"
#include <glm/glm.hpp>

namespace lve {

    class TerrainGenerator {
    public:
        TerrainGenerator(int seed = 1337) {
            noise_.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
            noise_.SetFrequency(0.01f);
            noise_.SetFractalType(FastNoiseLite::FractalType_FBm);
            noise_.SetFractalOctaves(4);
            noise_.SetFractalLacunarity(2.0f);
            noise_.SetFractalGain(0.5f);
            noise_.SetSeed(seed);
        }

        float getHeight(float x, float z) const {
            return noise_.GetNoise(x, z) * 10.0f;
        }

        static glm::vec3 getColor(float h) {
            float t = (h + 10.0f) / 20.0f;
            t = glm::clamp(t, 0.0f, 1.0f);

            glm::vec3 low{1.0f, 0.0f, 0.0f};
            glm::vec3 high{0.0f, 0.0f, 1.0f};

            return glm::mix(low, high, t);
        }

    private:
        FastNoiseLite noise_;
    };

} // namespace lve
