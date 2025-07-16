#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord; // Não usado, mas bom ter para o futuro

layout(location = 0) out vec4 outColor;

void main() {
    outColor = fragColor;
}