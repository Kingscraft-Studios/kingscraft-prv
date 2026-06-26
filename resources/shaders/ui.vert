#version 450

// TODO: SSBO Style System
// Binding 2: stylePool[64] — GpuStyle entries (3 x vec4 each)
// Binding 3: elementStyles[1024] — uint32_t styleIndex per elementId
// VS looks up both, passes flat style values to FS — zero FS SSBO reads.

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec4 inColor;
layout(location = 3) in uint inElementId;

layout(location = 0) out vec2 outUv;
layout(location = 1) flat out vec4 outColor1;
layout(location = 2) flat out vec4 outColor2;
layout(location = 3) flat out vec4 outParams;

layout(set = 0, binding = 0) uniform Proj {
    mat4 projection;
};

layout(set = 0, binding = 2) readonly buffer StylePool {
    vec4 pool[];
};

layout(set = 0, binding = 3) readonly buffer ElementStyles {
    uint styles[];
};

void main() {
    gl_Position = projection * vec4(inPos, 0.0, 1.0);
    outUv = inUv;

    uint styleIndex = styles[inElementId];

    // GpuStyle is 3 x vec4 per entry: color1, color2, params
    outColor1 = pool[styleIndex * 3 + 0];
    outColor2 = pool[styleIndex * 3 + 1];
    outParams = pool[styleIndex * 3 + 2];
}
