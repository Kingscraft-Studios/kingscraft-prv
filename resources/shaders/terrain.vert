#version 450

layout(push_constant) uniform PushConstants {
    mat4 modelViewProj;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;
layout(location = 2) in float inTexIndex;

layout(location = 0) out vec2 fragUv;
layout(location = 1) out float fragTexIndex;

void main() {
    gl_Position = pc.modelViewProj * vec4(inPosition, 1.0);
    fragUv = inUv;
    fragTexIndex = inTexIndex;
}
