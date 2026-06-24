#version 450

layout(location = 0) out vec2 outUv;

void main() {
    vec2 posUv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(posUv * 2.0 - 1.0, 0.0, 1.0);
    outUv = vec2(posUv.x, 1.0 - posUv.y);
}
