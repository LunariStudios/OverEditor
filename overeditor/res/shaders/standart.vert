#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 projection;
} vp;
layout(set = 1, binding = 0) uniform ModelBuffer {
    mat4 model;
} model;

void main() {
    gl_Position = vp.projection * vp.view * model.model * vec4(position, 1);
    fragColor = vec3(sin(position.x), sin(position.y), sin(position.z));
}