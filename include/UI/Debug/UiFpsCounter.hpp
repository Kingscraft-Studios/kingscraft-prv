#pragma once

#include "UI/Elements/UiTextBlock.hpp"
#include "Util/TimeUtil.hpp"

namespace lve {

    class UiWrapper;

    class UiFpsCounter {
    public:
        void init(UiWrapper& ui);
        void update();
        void setCpuGpuTimes(double cpuMs, double gpuMs, double terrainMs,
                            double cpuTickMs, double cpuSubmitMs,
                            double frustumMs, double drawMs);
        void cleanup(UiWrapper& ui);

    private:
        UiTextBlock text_;
        UiTextBlock cpuBreakdown_;
        uint32_t styleIndex_ = 0;
        double lastTime_ = 0.0;
        double elapsed_ = 0.0;
        int frameCount_ = 0;
        double latestCpuMs_ = 0.0;
        double latestGpuMs_ = 0.0;
        double latestTerrainMs_ = 0.0;
        double latestCpuTickMs_ = 0.0;
        double latestCpuSubmitMs_ = 0.0;
        double latestFrustumMs_ = 0.0;
        double latestDrawMs_ = 0.0;
    };

} // namespace lve
