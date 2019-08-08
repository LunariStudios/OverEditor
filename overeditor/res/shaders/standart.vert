#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform CameraMatrices {
    mat4 view;
    mat4 projection;
} matrices;

layout(set = 1, binding = 0) uniform ModelMatrix {
    mat4 value;
}model;

void main() {
    gl_Position = model.value * matrices.view * matrices.projection * vec4(position, 1);
    fragColor = vec3(1, 0, 0);
}