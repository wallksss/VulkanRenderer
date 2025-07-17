#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform Light {
    vec3 position;
    float radius;
    float intensity;
} light;

void main() {
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 - smoothstep(0.0, light.radius, distance);
    attenuation *= light.intensity;

    vec3 lightDir = normalize(light.position - fragPos);
    vec3 normal = normalize(fragNormal);
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
    
    vec3 ambient = 0.1 * vec3(1.0, 1.0, 1.0);
    
    vec3 result = (ambient + diffuse) * fragColor.rgb;
    result *= attenuation;
    outColor = vec4(result, fragColor.a);
}