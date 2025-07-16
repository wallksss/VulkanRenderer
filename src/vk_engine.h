#pragma once

#include <vector>
#include <optional>
#include <string>
#include <unordered_map>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "mesh.h"
#include "initializers.h"

const float BALL_RADIUS = 0.057f;

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

struct UniformBufferObject {
    // alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

//gihtheghost

//vai armazenar a cor
struct Material {
	glm::vec3 color{1.0f, 1.0f, 1.0f};
};

// Representa uma única chamada de desenho dentro de uma malha maior.
// Contém a informação de qual parte do buffer de índices usar e com qual material.
struct SubMesh {
    std::string materialName;
    uint32_t indexCount;
    uint32_t firstIndex; // O offset para o primeiro índice desta sub-malha no buffer de índices principal
};

// associa a geometria com os seus identificadores no buffer da GPU
struct Mesh {
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
    VkBuffer _vertexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory _vertexBufferMemory{VK_NULL_HANDLE};
    VkBuffer _indexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory _indexBufferMemory{VK_NULL_HANDLE};
    // Lista de partes desenháveis, divide a mesh em materiais
    std::vector<SubMesh> _subMeshes;
};

// determina como será o objeto que deve ser desenhado na cena, determinando qual
// malha e material usar e onde vai ser posicionado
struct RenderObject {
    std::string meshName;
    glm::mat4 transformMatrix;
};

// diz para a GPU como o objeto deve ser desenhado com sua cor e matriz de transformação
struct GPUDrawPushConstants {
    glm::mat4 transform;
    glm::vec4 color;
};

class VulkanApplication {
    public:
    void init();
    void run();
    void cleanup();
    
    private:

    const float MODEL_SCALE = 1.0f;

    void initWindow();
    void initVulkan();
    void mainLoop();
    
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createCommandPool();
    void createDepthResources();
    void createFramebuffers();

    // ---------------- funções para organização e carregamento de objetos ----------------- //

    void load_model(const char* filename); // carrega objeto e mtl
    void create_mesh_buffers(Mesh& mesh);  // cria buffers para uma malha
    void setup_scene();                   // cria lista de objetos da cena para renderizar
    void update_scene(float deltaTime);    // atualiza a posição dos objetos na cena a cada frame
    void create_debug_axes();             // cria eixos de depuração para visualização

    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffer();
    void createSyncObjects();
    
    void drawFrame();
    void updateUniformBuffer(uint32_t currentImage);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    
    void cleanupSwapChain();
    void recreateSwapChain();
    
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);
    
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    
    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkSurfaceKHR surface;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    

    // --- "banco de dados" para armazenar objetos e materiais --- //
    std::unordered_map<std::string, Material> _materials; //armazena os materias
    std::unordered_map<std::string, Mesh> _meshes; // armazena as malhas
    std::vector<RenderObject> _staticRenderables;  // Para mesa, lâmpada, etc.
    std::vector<RenderObject> _dynamicRenderables; // Para bolas, taco

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    


    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    
    uint32_t currentFrame = 0;
    bool framebufferResized = false;
    
    void setupPoolTable();
    void processInput(float deltaTime);
    void updatePhysics(float deltaTime);
    std::vector<PoolBall> balls;
    CueStick cue;
    
    glm::vec2 table_min_bounds;
    glm::vec2 table_max_bounds;

    // Camera state for orbiting
    float cameraYaw = glm::radians(45.0f);
    float cameraPitch = glm::radians(45.0f);
    float cameraDistance = 5.0f;
};



