#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(push_constant) uniform Push {
    vec2 screenSize;
} push;

void main() {
    vec2 pos = (position / push.screenSize) * 2.0 - 1.0;

    gl_Position = vec4(pos, 0.0, 1.0);

    fragColor = color;
    fragTexCoord = texCoord;
}