#pragma once

#include <vulkan/vulkan.h>

/*
#include <vector> // Certifique-se de que <vector> está incluído
#include <optional> // Certifique-se de que <optional> está incluído
#include <string> // Certifique-se de que <string> está incluído
#include <unordered_map> // Certifique-se de que <unordered_map> está incluído

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

// Inclua mesh.h e initializers.h se ainda não estiverem aqui
#include "mesh.h"
#include "initializers.h"

const float BALL_RADIUS = 0.16f;

struct PoolBall {
    int id;
    glm::vec2 position;
    glm::vec2 velocity;
    float radius;
    bool is_moving;
};

struct CueStick {
    glm::vec2 position;
    float angle;
    float power;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct PointLight {
    alignas(16) glm::vec3 position; // Posição da luz no espaço do mundo
    alignas(16) glm::vec3 color;    // Cor da luz (RGB)
    alignas(4) float intensity;     // Intensidade da luz
    // Você pode adicionar outros parâmetros como atenuação, etc.
};

struct LightUBO {
    alignas(16) PointLight lampLight; // A luz da sua lâmpada
    // Se tiver mais luzes, pode adicionar aqui
};

struct UniformBufferObject {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

*/
