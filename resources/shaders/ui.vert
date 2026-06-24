#version 450

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec2 outUv;
layout(location = 1) out vec4 outColor;

layout(set = 0, binding = 0) uniform Proj {
    mat4 projection;
};

void main() {
    gl_Position = projection * vec4(inPos, 0.0, 1.0);
    outUv = inUv;
    outColor = inColor;
}
