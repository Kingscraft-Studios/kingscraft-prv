#pragma once

#include "Core/Resources/BlockModel.hpp"

namespace lve {

class Block {
protected:
    BlockModel model_;
    int textureBaseOffset_ = 0;
public:
    Block() = default;
    explicit Block(BlockModel model) : model_(std::move(model)) {}
    virtual ~Block() = default;

    const BlockModel& getModel() const { return model_; }

    int getTextureBaseOffset() const { return textureBaseOffset_; }
    void setTextureBaseOffset(int offset) { textureBaseOffset_ = offset; }

    virtual bool isSolid() const { return true; }
    virtual bool isTransparent() const { return false; }
    virtual bool isOpaque() const { return isSolid() && !isTransparent(); }
    virtual bool isLiquid() const { return false; }
    virtual bool isReplaceable() const { return false; }
    virtual int getLightEmission() const { return 0; }
    virtual int getLightAbsorption() const { return isOpaque() ? 15 : 0; }
    virtual float getHardness() const { return 1.0f; }
};

} // namespace lve
