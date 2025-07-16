#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord; // Corrigido para location = 1

// UBO para matrizes de câmera (view, projection)
layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

// Push Constants para dados por objeto (transformação, cor)
layout(push_constant) uniform constants {
    mat4 transform;
    vec4 color;
} push;

// Saída para o Fragment Shader
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * push.transform * vec4(inPosition, 1.0);
    fragColor = push.color; // Passa a cor do material para o fragment shader
    fragTexCoord = inTexCoord;
}