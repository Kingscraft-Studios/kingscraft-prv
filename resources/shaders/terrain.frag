#version 450

layout(binding = 0) uniform sampler2DArray uTexture;

layout(location = 0) in vec2 fragUv;
layout(location = 1) in float fragTexIndex;

layout(location = 0) out vec4 outColor;

void main() {
    int layer = int(fragTexIndex);
    outColor = texture(uTexture, vec3(fragUv, layer));
}
