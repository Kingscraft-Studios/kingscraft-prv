#pragma once

#include "Device.hpp"
#include <string>

namespace lve {

    class Texture {
    public:
        Texture(Device &device, const std::string &filepath);

        ~Texture();

        VkImageView getImageView() const { return textureImageView; }

        VkSampler getSampler() const { return textureSampler; }

    private:
        void createTextureImage(const std::string &filepath);

        void createTextureImageView();

        void createTextureSampler();

        Device &device;

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
    };

}  // namespace lve
