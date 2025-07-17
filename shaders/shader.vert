#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform constants {
    mat4 transform;
    vec4 color;
} push;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec4 fragColor;

void main() {
    vec4 worldPosition = push.transform * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * worldPosition;
    
    fragPos = vec3(worldPosition);
    fragNormal = mat3(transpose(inverse(push.transform))) * inNormal;
    fragColor = push.color;
}