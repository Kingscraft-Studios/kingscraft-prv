#version 450

layout(location = 0) in vec2 inUv;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D offscreenTex;

void main() {
    outColor = texture(offscreenTex, inUv);
}
