#version 450

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    // Full-screen triangle covering the entire viewport
    // No vertex inputs needed — positions generated from gl_VertexIndex
    vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );
    vec2 pos = positions[gl_VertexIndex];
    gl_Position = vec4(pos, 0.0, 1.0);
    fragColor = vec3(1.0);
    fragTexCoord = pos * 0.5 + 0.5;
}
