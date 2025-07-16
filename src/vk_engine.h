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

const float BALL_RADIUS = 0.16f;

struct PoolBall {
    int id;
    glm::vec2 position;
    glm::vec2 velocity;
    float radius;
    bool is_moving;
    glm::quat rotation;
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
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct Material {
	glm::vec3 color{1.0f, 1.0f, 1.0f};
};

struct SubMesh {
    std::string materialName;
    uint32_t indexCount;
    uint32_t firstIndex;
};

struct Mesh {
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
    VkBuffer _vertexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory _vertexBufferMemory{VK_NULL_HANDLE};
    VkBuffer _indexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory _indexBufferMemory{VK_NULL_HANDLE};
    std::vector<SubMesh> _subMeshes;
};

struct RenderObject {
    std::string meshName;
    glm::mat4 transformMatrix;
};

struct GPUDrawPushConstants {
    glm::mat4 transform;
    glm::vec4 color;
};

class VulkanApplication {
    public:
    
    // Initializes the Vulkan application, including window creation and Vulkan setup.
    void init();
    // Starts the main application loop.
    void run();
    // Cleans up all Vulkan resources and terminates the application.
    void cleanup();
    
    private:

    const float MODEL_SCALE = 1.0f;

    // Initializes the GLFW window.
    void initWindow();
    // Initializes the core Vulkan components.
    void initVulkan();
    // The main application loop where frames are rendered and events are processed.
    void mainLoop();
    
    // Creates the Vulkan instance.
    void createInstance();
    // Sets up the debug messenger for validation layer messages.
    void setupDebugMessenger();
    // Creates the window surface for rendering.
    void createSurface();
    // Selects a suitable physical device (GPU).
    void pickPhysicalDevice();
    // Creates the logical device for interacting with the GPU.
    void createLogicalDevice();
    // Creates the swap chain for presenting images to the screen.
    void createSwapChain();
    // Creates image views for the swap chain images.
    void createImageViews();
    // Creates the render pass that defines the rendering operations.
    void createRenderPass();
    // Creates the descriptor set layout for uniform buffers.
    void createDescriptorSetLayout();
    // Creates the graphics pipeline, including shaders and vertex formats.
    void createGraphicsPipeline();
    // Creates the command pool for allocating command buffers.
    void createCommandPool();
    // Creates the depth resources for depth testing.
    void createDepthResources();
    // Creates the framebuffers for the swap chain.
    void createFramebuffers();

    // Loads a 3D model from an OBJ file.
    void load_model(const char* filename);
    // Creates the vertex and index buffers for a given mesh.
    void create_mesh_buffers(Mesh& mesh);
    // Sets up the initial scene with all objects.
    void setup_scene();
    // Updates the scene's dynamic objects each frame.
    void update_scene(float deltaTime);
    // Creates debug axes for visualization.
    void create_debug_axes();
    // Draws a vertical line for debugging purposes.
    void draw_debug_vertical_line(glm::vec2 xz_pos, float height, const std::string& name, const std::string& materialName);
    // Creates debug lines to visualize the table boundaries.
    void create_debug_bounds_lines();

    // Creates the uniform buffers for passing data to shaders.
    void createUniformBuffers();
    // Creates the descriptor pool for allocating descriptor sets.
    void createDescriptorPool();
    // Creates the descriptor sets for the uniform buffers.
    void createDescriptorSets();
    // Creates the command buffers for recording rendering commands.
    void createCommandBuffer();
    // Creates synchronization objects like semaphores and fences.
    void createSyncObjects();
    
    // Renders a single frame.
    void drawFrame();
    // Updates the uniform buffer with the current camera matrices.
    void updateUniformBuffer(uint32_t currentImage);
    // Records the rendering commands into a command buffer.
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    
    // Cleans up the swap chain and its associated resources.
    void cleanupSwapChain();
    // Recreates the swap chain when the window is resized.
    void recreateSwapChain();
    
    // Populates the debug messenger create info structure.
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    // Checks if the requested validation layers are available.
    bool checkValidationLayerSupport();
    // Gets the required instance extensions for Vulkan.
    std::vector<const char*> getRequiredExtensions();
    // Checks if a given physical device is suitable for the application.
    bool isDeviceSuitable(VkPhysicalDevice device);
    // Checks if a given physical device supports the required extensions.
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    // Finds the queue families for a given physical device.
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    // Queries the swap chain support details for a given physical device.
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    // Chooses the swap surface format for the swap chain.
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    // Chooses the present mode for the swap chain.
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    // Chooses the swap extent for the swap chain.
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    // Creates a shader module from SPIR-V code.
    VkShaderModule createShaderModule(const std::vector<char>& code);
    // Finds a suitable memory type for a given buffer.
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    // Creates a buffer and allocates its memory.
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    // Copies data from one buffer to another.
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    // Begins a single-time command buffer.
    VkCommandBuffer beginSingleTimeCommands();
    // Ends and submits a single-time command buffer.
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    // Transitions the layout of an image.
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    // Copies data from a buffer to an image.
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    // Creates an image and allocates its memory.
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    // Creates an image view for a given image.
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    // Finds a supported format from a list of candidates.
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    // Finds the depth format for the depth buffer.
    VkFormat findDepthFormat();
    // Checks if a given format has a stencil component.
    bool hasStencilComponent(VkFormat format);
    
    // Callback function for window resize events.
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    // Callback function for Vulkan validation layer messages.
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
    
    std::unordered_map<std::string, Material> _materials;
    std::unordered_map<std::string, Mesh> _meshes;
    std::vector<RenderObject> _staticRenderables;
    std::vector<RenderObject> _dynamicRenderables;

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

    float cameraYaw = glm::radians(60.0f);
    float cameraPitch = glm::radians(45.0f);
    float cameraDistance = 15.0f;
};