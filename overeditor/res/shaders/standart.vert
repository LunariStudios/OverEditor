#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Matrices {
    mat4 mvp;
} matrices;

void main() {
    gl_Position = matrices.mvp * vec4(position, 1);
    fragColor = vec3(1, 0, 0);
}