#pragma once
#include <NsGui/MemoryStream.h>

namespace lve {
    class MemoryStream : public Noesis::MemoryStream {
    public:
        MemoryStream(void* buffer, uint32_t size)
            : Noesis::MemoryStream(buffer, size), mOwned(buffer) {}

        ~MemoryStream() override {
            free(mOwned);
        }

    private:
        void* mOwned;
    };
}