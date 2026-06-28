#pragma once

#include "UI/Elements/UiTextBlock.hpp"
#include "Util/TimeUtil.hpp"

namespace lve {

    class UiWrapper;

    class UiFpsCounter {
    public:
        void init(UiWrapper& ui);
        void update();
        void setCpuGpuTimes(double cpuMs, double gpuMs);
        void cleanup(UiWrapper& ui);

    private:
        UiTextBlock text_;
        uint32_t styleIndex_ = 0;
        double lastTime_ = 0.0;
        double elapsed_ = 0.0;
        int frameCount_ = 0;
        double latestCpuMs_ = 0.0;
        double latestGpuMs_ = 0.0;
    };

} // namespace lve
