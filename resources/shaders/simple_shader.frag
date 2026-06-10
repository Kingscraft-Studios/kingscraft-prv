#version 450

layout(location = 0) in vec3 fragColor;         // interpolated color from vertex
layout(location = 1) in vec2 fragTexCoord;      // interpolated UVs

layout(location = 0) out vec4 outColor;         // output to framebuffer

layout(set = 0, binding = 0) uniform sampler2D texSampler;

void main() {
    vec4 texColor = texture(texSampler, fragTexCoord);
    outColor = texColor * vec4(fragColor, 1.0);  // modulate texture by vertex color
}
