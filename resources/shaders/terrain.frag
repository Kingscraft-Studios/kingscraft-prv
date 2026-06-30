#version 450

layout(constant_id = 0) const bool DISABLE_TEXTURES = false;

layout(binding = 0) uniform sampler2DArray uTexture;

layout(location = 0) in vec2 fragUv;
layout(location = 1) in float fragTexIndex;

layout(location = 0) out vec4 outColor;

void main() {
    if (DISABLE_TEXTURES) {
        outColor = vec4(1.0, 0.0, 0.0, 1.0);
    } else {
        int layer = int(fragTexIndex);
        outColor = texture(uTexture, vec3(fragUv, layer));
    }
}
