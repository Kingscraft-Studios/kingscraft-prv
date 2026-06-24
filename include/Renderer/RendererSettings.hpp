#pragma once

namespace lve {

    class RendererSettings {
    public:
        static RendererSettings& get() {
            static RendererSettings instance;
            return instance;
        }

        // World / terrain
        int renderDistance = 10;

        // Camera
        float fov = 70.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;

        // Performance toggles
        bool vsync = false;
        bool enableFrustumCulling = true;
        int maxFps = 0; // 0 = None

    private:
        RendererSettings() = default;
    };

} // namespace lve
