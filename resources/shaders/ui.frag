#version 450

layout(location = 0) in vec2 inUv;
layout(location = 1) in vec4 inColor;

layout(set = 0, binding = 1) uniform sampler2D fontAtlas;

layout(location = 0) out vec4 outColor;

void main() {
    if (inUv.x < 0.0) {
        outColor = inColor;
    } else {
        float alpha = texture(fontAtlas, inUv).a;
        outColor = vec4(inColor.rgb, inColor.a * alpha);
    }
}
