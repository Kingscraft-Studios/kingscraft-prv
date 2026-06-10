#pragma once

#include <NoesisPCH.h>
#include <string>

#include "MemoryStream.hpp"

namespace lve {
    class XamlProvider : public Noesis::XamlProvider {
    public:
        // In XamlProvider.hpp
        Noesis::Ptr<Noesis::Stream> LoadXaml(const Noesis::Uri& uri) override {
            std::string path = "resources/ui/" + std::string(uri.Str());

            FILE* file = fopen(path.c_str(), "rb");
            if (!file) return nullptr;

            fseek(file, 0, SEEK_END);
            size_t size = ftell(file);
            rewind(file);

            void* buffer = malloc(size);
            if (fread(buffer, 1, size, file) != size) {
                free(buffer);
                fclose(file);
                return nullptr;
            }
            fclose(file);

            // Use Noesis::MakePtr to ensure the reference count starts at 1
            // and Noesis handles the 'delete' later.
            return Noesis::MakePtr<MemoryStream>(buffer, (uint32_t)size);
        }
    };
}
