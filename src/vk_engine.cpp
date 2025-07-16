#include "vk_engine.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "models/sphere.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";
const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    
    if(!file.is_open()) {
        throw std::runtime_error("failed to open file");
    }
    
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            size_t h1 = hash<glm::vec3>()(vertex.pos);
            size_t h2 = hash<glm::vec2>()(vertex.texCoord);
            return h1 ^ (h2 << 1);
        }
    };
}

void VulkanApplication::init() {
    initWindow();
    initVulkan();
}

void VulkanApplication::run() {
    mainLoop();
}

void VulkanApplication::cleanup() {
    cleanupSwapChain();
    
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    
    for (auto const& [name, mesh] : _meshes) {
        vkDestroyBuffer(device, mesh._vertexBuffer, nullptr);
        vkFreeMemory(device, mesh._vertexBufferMemory, nullptr);
        vkDestroyBuffer(device, mesh._indexBuffer, nullptr);
        vkFreeMemory(device, mesh._indexBufferMemory, nullptr);
    }

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    
    for(size_t i = 0; i < swapChainImages.size(); i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
    }
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
    
    vkDestroyCommandPool(device, commandPool, nullptr);    
    vkDestroyDevice(device, nullptr);
    
    if(enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

void VulkanApplication::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Pool", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void VulkanApplication::setupPoolTable() {
    balls.clear();

    glm::vec2 original_cue_pos = {0.0f, -0.8f};
    balls.push_back({0, {-original_cue_pos.y, original_cue_pos.x}, {0.0f, 0.0f}, BALL_RADIUS, false, glm::quat(1.0f, 0.0f, 0.0f, 0.0f)});

    int ball_id = 1;
    float rack_start_x = 0.0f;
    float rack_y_offset = 5.0f;

    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col <= row; ++col) {
            float x = rack_start_x + col * (2.0f * BALL_RADIUS) - row * BALL_RADIUS;
            float y = rack_y_offset + row * (2.0f * BALL_RADIUS * 0.866f);

            glm::vec2 rotated_pos = {-y, x};
            balls.push_back({ball_id++, rotated_pos, {0.0f, 0.0f}, BALL_RADIUS, false, glm::quat(1.0f, 0.0f, 0.0f, 0.0f)});
        }
    }

    cue.angle = 0.0f;
    cue.power = 8.0f;

    table_min_bounds = {-63.0f * BALL_RADIUS, -25.0f * BALL_RADIUS};
    table_max_bounds = {21.0f * BALL_RADIUS, 25.0f * BALL_RADIUS};
}

void VulkanApplication::processInput(float deltaTime) {
    float rotationSpeed = 2.0f * deltaTime;
    float zoomSpeed = 5.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraYaw -= rotationSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraYaw += rotationSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPitch -= rotationSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPitch += rotationSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        cameraDistance -= zoomSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        cameraDistance += zoomSpeed;
    }

    cameraPitch = glm::clamp(cameraPitch, glm::radians(5.0f), glm::radians(85.0f));
    cameraDistance = glm::clamp(cameraDistance, 2.0f, 50.0f);

    if (balls[0].is_moving) return;

    float cueRotationSpeed = 2.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        cue.angle -= cueRotationSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        cue.angle += cueRotationSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        glm::vec2 direction = {-1 * glm::cos(cue.angle), glm::sin(cue.angle)};
        balls[0].velocity = direction * cue.power;
        balls[0].is_moving = true;
    }
}


void VulkanApplication::updatePhysics(float deltaTime) {
    float friction = 0.5f;
    
    bool any_ball_is_moving = false;
    
    for (auto& ball : balls) {
        if (glm::length(ball.velocity) > 0.015f) {
            glm::vec2 displacement = ball.velocity * deltaTime;
            ball.position += displacement;

            float distance = glm::length(displacement);
            glm::vec3 rotation_axis = glm::vec3(displacement.y, 0.0f, -displacement.x);
            float rotation_angle = distance / ball.radius;

            glm::quat rotation_delta = glm::angleAxis(rotation_angle, glm::normalize(rotation_axis));
            ball.rotation = rotation_delta * ball.rotation;
            
            ball.velocity -= ball.velocity * friction * deltaTime;
            
            ball.is_moving = true;
            any_ball_is_moving = true;
        } else {
            ball.velocity = {0.0f, 0.0f};
            ball.is_moving = false;
        }
    }
    
    if (!any_ball_is_moving) {
        balls[0].is_moving = false;
    }
    
    for (size_t i = 0; i < balls.size(); ++i) {
        auto& ball = balls[i];
        if (ball.position.x - ball.radius < table_min_bounds.x) {
            if (i == 0) {
                std::cout << "[DEBUG] Cue Ball Collision: Left Wall" << std::endl;
                std::cout << "        Position X: " << ball.position.x - ball.radius << " | Boundary: " << table_min_bounds.x << std::endl;
            }
            ball.position.x = table_min_bounds.x + ball.radius;
            ball.velocity.x *= -1;
        }
        if (ball.position.x + ball.radius > table_max_bounds.x) {
            if (i == 0) {
                std::cout << "[DEBUG] Cue Ball Collision: Right Wall" << std::endl;
                std::cout << "        Position X: " << ball.position.x + ball.radius << " | Boundary: " << table_max_bounds.x << std::endl;
            }
            ball.position.x = table_max_bounds.x - ball.radius;
            ball.velocity.x *= -1;
        }
        if (ball.position.y - ball.radius < table_min_bounds.y) {
            if (i == 0) {
                std::cout << "[DEBUG] Cue Ball Collision: Bottom Wall" << std::endl;
                std::cout << "        Position Y: " << ball.position.y - ball.radius << " | Boundary: " << table_min_bounds.y << std::endl;
            }
            ball.position.y = table_min_bounds.y + ball.radius;
            ball.velocity.y *= -1;
        }
        if (ball.position.y + ball.radius > table_max_bounds.y) {
            if (i == 0) {
                std::cout << "[DEBUG] Cue Ball Collision: Top Wall" << std::endl;
                std::cout << "        Position Y: " << ball.position.y + ball.radius << " | Boundary: " << table_max_bounds.y << std::endl;
            }
            ball.position.y = table_max_bounds.y - ball.radius;
            ball.velocity.y *= -1;
        }
    }
    
    for (size_t i = 0; i < balls.size(); ++i) {
        for (size_t j = i + 1; j < balls.size(); ++j) {
            PoolBall& b1 = balls[i];
            PoolBall& b2 = balls[j];
            
            glm::vec2 delta = b2.position - b1.position;
            float dist_sq = glm::dot(delta, delta);
            float min_dist = b1.radius + b2.radius;
            
            if (dist_sq < min_dist * min_dist) {
                float dist = sqrt(dist_sq);
                glm::vec2 normal = (dist > 0) ? delta / dist : glm::vec2(1, 0);
                
                float overlap = min_dist - dist;
                b1.position -= normal * (overlap / 2.0f);
                b2.position += normal * (overlap / 2.0f);
                
                glm::vec2 tangent = {-normal.y, normal.x};
                
                float v1n = glm::dot(b1.velocity, normal);
                float v1t = glm::dot(b1.velocity, tangent);
                float v2n = glm::dot(b2.velocity, normal);
                float v2t = glm::dot(b2.velocity, tangent);
                
                b1.velocity = tangent * v1t + normal * v2n;
                b2.velocity = tangent * v2t + normal * v1n;
                
                b1.is_moving = true;
                b2.is_moving = true;
            }
        }
    }
}

void VulkanApplication::create_mesh_buffers(Mesh& mesh) {
    VkDeviceSize vertexBufferSize = sizeof(mesh._vertices[0]) * mesh._vertices.size();

    VkBuffer vertexStagingBuffer;
    VkDeviceMemory vertexStagingBufferMemory;
    createBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 vertexStagingBuffer, vertexStagingBufferMemory);

    void* data;
    vkMapMemory(device, vertexStagingBufferMemory, 0, vertexBufferSize, 0, &data);
    memcpy(data, mesh._vertices.data(), (size_t)vertexBufferSize);
    vkUnmapMemory(device, vertexStagingBufferMemory);

    createBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 mesh._vertexBuffer, mesh._vertexBufferMemory);

    copyBuffer(vertexStagingBuffer, mesh._vertexBuffer, vertexBufferSize);

    vkDestroyBuffer (device, vertexStagingBuffer, nullptr);
    vkFreeMemory(device, vertexStagingBufferMemory, nullptr);

    VkDeviceSize indexBufferSize = sizeof(mesh._indices[0]) * mesh._indices.size();
    VkBuffer indexStagingBuffer;
    VkDeviceMemory indexStagingBufferMemory;
    createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexStagingBuffer, indexStagingBufferMemory);

    vkMapMemory(device, indexStagingBufferMemory, 0, indexBufferSize, 0, &data);
    memcpy(data, mesh._indices.data(), (size_t)indexBufferSize);
    vkUnmapMemory(device, indexStagingBufferMemory);

    createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mesh._indexBuffer, mesh._indexBufferMemory);
    copyBuffer(indexStagingBuffer, mesh._indexBuffer, indexBufferSize);

    vkDestroyBuffer(device, indexStagingBuffer, nullptr);
    vkFreeMemory(device, indexStagingBufferMemory, nullptr);

}

void VulkanApplication::load_model(const char* filename) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    const char* mtl_basedir = "textures/";

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, mtl_basedir, true)) {
        throw std::runtime_error(warn + err);
    }

    for (const auto& mat : materials) {
        if (_materials.find(mat.name) == _materials.end()) {
            Material newMaterial;
            newMaterial.color = {mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]};
            _materials[mat.name] = newMaterial;
        }
    }
    if (_materials.find("Default") == _materials.end()) {
        _materials["Default"] = Material{};
    }

    for (const auto& shape : shapes) {
        if (_meshes.count(shape.name)) continue;

        Mesh newMesh;
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        std::map<int, std::vector<uint32_t>> indices_by_material;

        for (size_t i = 0; i < shape.mesh.indices.size(); ++i) {
            size_t face_index = i / 3;
            int material_id = shape.mesh.material_ids[face_index];
            const auto& index = shape.mesh.indices[i];

            Vertex vertex{};
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if (index.texcoord_index >= 0) {
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(newMesh._vertices.size());
                newMesh._vertices.push_back(vertex);
            }

            indices_by_material[material_id].push_back(uniqueVertices[vertex]);
        }

        uint32_t currentIndexOffset = 0;
        for(auto const& [mat_id, mat_indices] : indices_by_material) {
            SubMesh submesh;
            submesh.firstIndex = currentIndexOffset;
            submesh.indexCount = static_cast<uint32_t>(mat_indices.size());

            if (mat_id >= 0) {
                submesh.materialName = materials[mat_id].name;
            } else {
                submesh.materialName = "Default";
            }

            newMesh._subMeshes.push_back(submesh);
            newMesh._indices.insert(newMesh._indices.end(), mat_indices.begin(), mat_indices.end());
            currentIndexOffset += submesh.indexCount;
        }

        if (newMesh._indices.empty()) {
            continue;
        }

        create_mesh_buffers(newMesh);
        _meshes[shape.name] = newMesh;
    }
}

struct AssetInfo {
    std::string objFilePath;
    std::string meshNameInObj;
};

void VulkanApplication::setup_scene() {
    std::cout << "[INFO] Setting up scene..." << std::endl;

    std::unordered_map<std::string, AssetInfo> assetInfoMap = {
        {"table",     {"models/pooltable.obj", "PoolTable"}},
        {"stick",     {"models/poolstick.obj", "PoolStick"}},
        {"lamp",      {"models/luz.obj",       "Light_Ceiling1"}},

        {"ball_0",    {"models/cueball.obj",   "cue_ball"}},
        {"ball_1",    {"models/amarela1.obj",  "all_balls.007"}},
        {"ball_2",    {"models/amarela2.obj",  "all_balls.012"}},
        {"ball_3",    {"models/azul1.obj",     "all_balls.008"}},
        {"ball_4",    {"models/azul2.obj",     "all_balls.005"}},
        {"ball_5",    {"models/laranja1.obj",  "all_balls.003"}},
        {"ball_6",    {"models/laranja2.obj",  "all_balls"}},
        {"ball_7",    {"models/preta.obj",     "all_balls.009"}},
        {"ball_8",    {"models/roxa1.obj",     "all_balls.002"}},
        {"ball_9",    {"models/roxa2.obj",     "all_balls.013"}},
        {"ball_10",   {"models/verde1.obj",    "all_balls.011"}},
        {"ball_11",   {"models/verde2.obj",    "all_balls.010"}},
        {"ball_12",   {"models/vermelha1.obj", "all_balls.001"}},
        {"ball_13",   {"models/vermelha2.obj", "all_balls.006"}},
        {"ball_14",   {"models/vinho1.obj",    "all_balls.004"}},
        {"ball_15",   {"models/vinho2.obj",    "all_balls.014"}}
    };

    std::cout << "[INFO] Loading models..." << std::endl;
    std::set<std::string> loadedFiles;
    for (const auto& pair : assetInfoMap) {
        const AssetInfo& assetInfo = pair.second;
        if (loadedFiles.find(assetInfo.objFilePath) == loadedFiles.end()) {
            std::cout << "       - Loading file: " << assetInfo.objFilePath << std::endl;
            load_model(assetInfo.objFilePath.c_str());
            loadedFiles.insert(assetInfo.objFilePath);
        }
    }
    std::cout << "[INFO] Models loaded. Total meshes: " << _meshes.size() << std::endl;

    _staticRenderables.clear();
    _dynamicRenderables.clear();

    glm::vec3 scale_vector(MODEL_SCALE);

    RenderObject table_object;
    table_object.meshName = assetInfoMap["table"].meshNameInObj;
    if (_meshes.find(table_object.meshName) == _meshes.end()) {
         std::cerr << "[ERROR] Table mesh not found: " << table_object.meshName << std::endl;
    }
    glm::mat4 table_translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 table_scale = glm::scale(glm::mat4(1.0f), scale_vector);
    table_object.transformMatrix = table_translation * table_scale;
    _staticRenderables.push_back(table_object);
	
	// LÂMPADA
    RenderObject lamp_object;
    lamp_object.meshName = assetInfoMap["lamp"].meshNameInObj;
    if (_meshes.find(lamp_object.meshName) == _meshes.end()) {
         std::cerr << "[ERROR] Lamp mesh not found: " << lamp_object.meshName << std::endl;
    }
    glm::mat4 lamp_translation = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 5.8f, 0.0f));
    glm::vec3 new_scale_vector = glm::vec3(3.5f);
    glm::mat4 lamp_scale = glm::scale(glm::mat4(1.0f), new_scale_vector);
    lamp_object.transformMatrix = lamp_translation * lamp_scale;
    _staticRenderables.push_back(lamp_object);
    std::cout << "[INFO] Static renderables added." << std::endl;

    for (const auto& ball_data : balls) {
        RenderObject ball_object;
        std::string ball_key = "ball_" + std::to_string(ball_data.id);
        auto it = assetInfoMap.find(ball_key);

        if (it != assetInfoMap.end()) {
            ball_object.meshName = it->second.meshNameInObj;
            if (_meshes.find(ball_object.meshName) == _meshes.end()) {
                 std::cerr << "[ERROR] Ball mesh not found: " << ball_object.meshName << " (key: " << ball_key << ")" << std::endl;
                 continue;
            }
            ball_object.transformMatrix = glm::scale(glm::mat4(1.0f), scale_vector);
            _dynamicRenderables.push_back(ball_object);
        } else {
            std::cerr << "[ERROR] Asset info for '" << ball_key << "' not found!" << std::endl;
        }
    }
    std::cout << "[INFO] Ball renderables added. Total: " << _dynamicRenderables.size() << std::endl;

    RenderObject cue_object;
    auto stick_it = assetInfoMap.find("stick");
    if (stick_it != assetInfoMap.end()) {
        cue_object.meshName = stick_it->second.meshNameInObj;
        if (_meshes.find(cue_object.meshName) == _meshes.end()) {
            std::cerr << "[ERROR] Cue stick mesh not found: " << cue_object.meshName << std::endl;
        } else {
            cue_object.transformMatrix = glm::scale(glm::mat4(1.0f), scale_vector);
            _dynamicRenderables.push_back(cue_object);
            std::cout << "[INFO] Cue stick renderable added." << std::endl;
        }
    }

    std::cout << "[INFO] Scene setup complete. Dynamic renderables: " << _dynamicRenderables.size() << " | Physics balls: " << balls.size() << std::endl;
}

void VulkanApplication::create_debug_axes() {
    _materials["debug_red"] = Material{{0.8f, 0.1f, 0.1f}};
    _materials["debug_green"] = Material{{0.1f, 0.8f, 0.1f}};
    _materials["debug_blue"] = Material{{0.1f, 0.1f, 0.8f}};

    const float length = 1.5f;
    const float width = 0.05f;

    {
        Mesh xAxisMesh;
        xAxisMesh._vertices.push_back({{0, -width, 0}, {0,0}});
        xAxisMesh._vertices.push_back({{0, width, 0}, {0,0}});
        xAxisMesh._vertices.push_back({{length, width, 0}, {0,0}});
        xAxisMesh._vertices.push_back({{length, -width, 0}, {0,0}});
        xAxisMesh._indices = {0, 1, 2, 0, 2, 3};
        
        SubMesh submesh;
        submesh.materialName = "debug_red";
        submesh.firstIndex = 0;
        submesh.indexCount = 6;
        xAxisMesh._subMeshes.push_back(submesh);

        create_mesh_buffers(xAxisMesh);
        _meshes["debug_axis_x"] = xAxisMesh;

        RenderObject axis_x_object;
        axis_x_object.meshName = "debug_axis_x";
        axis_x_object.transformMatrix = glm::mat4(1.0f);
        _staticRenderables.push_back(axis_x_object);
    }

    {
        Mesh yAxisMesh;
        yAxisMesh._vertices.push_back({{-width, 0, 0}, {0,0}});
        yAxisMesh._vertices.push_back({{width, 0, 0}, {0,0}});
        yAxisMesh._vertices.push_back({{width, length, 0}, {0,0}});
        yAxisMesh._vertices.push_back({{-width, length, 0}, {0,0}});
        yAxisMesh._indices = {0, 1, 2, 0, 2, 3};

        SubMesh submesh;
        submesh.materialName = "debug_green";
        submesh.firstIndex = 0;
        submesh.indexCount = 6;
        yAxisMesh._subMeshes.push_back(submesh);

        create_mesh_buffers(yAxisMesh);
        _meshes["debug_axis_y"] = yAxisMesh;

        RenderObject axis_y_object;
        axis_y_object.meshName = "debug_axis_y";
        axis_y_object.transformMatrix = glm::mat4(1.0f);
        _staticRenderables.push_back(axis_y_object);
    }

    {
        Mesh zAxisMesh;
        zAxisMesh._vertices.push_back({{-width, 0, 0}, {0,0}});
        zAxisMesh._vertices.push_back({{width, 0, 0}, {0,0}});
        zAxisMesh._vertices.push_back({{width, 0, length}, {0,0}});
        zAxisMesh._vertices.push_back({{-width, 0, length}, {0,0}});
        zAxisMesh._indices = {0, 1, 2, 0, 2, 3};

        SubMesh submesh;
        submesh.materialName = "debug_blue";
        submesh.firstIndex = 0;
        submesh.indexCount = 6;
        zAxisMesh._subMeshes.push_back(submesh);

        create_mesh_buffers(zAxisMesh);
        _meshes["debug_axis_z"] = zAxisMesh;

        RenderObject axis_z_object;
        axis_z_object.meshName = "debug_axis_z";
        axis_z_object.transformMatrix = glm::mat4(1.0f);
        _staticRenderables.push_back(axis_z_object);
    }
}

void VulkanApplication::draw_debug_vertical_line(glm::vec2 xz_pos, float height, const std::string& name, const std::string& materialName) {
    const float width = 0.02f;

    Mesh lineMesh;
    lineMesh._vertices.push_back({{xz_pos.x - width, 0.0f, xz_pos.y}, {0,0}});
    lineMesh._vertices.push_back({{xz_pos.x + width, 0.0f, xz_pos.y}, {0,0}});
    lineMesh._vertices.push_back({{xz_pos.x + width, height, xz_pos.y}, {0,0}});
    lineMesh._vertices.push_back({{xz_pos.x - width, height, xz_pos.y}, {0,0}});
    lineMesh._indices = {0, 1, 2, 0, 2, 3};

    SubMesh submesh;
    submesh.materialName = materialName;
    submesh.firstIndex = 0;
    submesh.indexCount = 6;
    lineMesh._subMeshes.push_back(submesh);

    create_mesh_buffers(lineMesh);
    _meshes[name] = lineMesh;

    RenderObject line_object;
    line_object.meshName = name;
    line_object.transformMatrix = glm::mat4(1.0f);
    _staticRenderables.push_back(line_object);
}

void VulkanApplication::create_debug_bounds_lines() {
    _materials["debug_white"] = Material{{1.0f, 1.0f, 1.0f}};

    const float lineHeight = 2.0f;

    glm::vec2 min = table_min_bounds;
    glm::vec2 max = table_max_bounds;

    glm::vec2 p1 = {min.x, min.y};
    glm::vec2 p2 = {max.x, min.y};
    glm::vec2 p3 = {max.x, max.y};
    glm::vec2 p4 = {min.x, max.y};

    draw_debug_vertical_line(p1, lineHeight, "debug_line_1", "debug_white");
    draw_debug_vertical_line(p2, lineHeight, "debug_line_2", "debug_white");
    draw_debug_vertical_line(p3, lineHeight, "debug_line_3", "debug_white");
    draw_debug_vertical_line(p4, lineHeight, "debug_line_4", "debug_white");
}

void VulkanApplication::update_scene(float deltaTime) {
    glm::vec3 scale_vector(MODEL_SCALE);
    glm::mat4 y_to_z_up_rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    if (_dynamicRenderables.size() != balls.size() + 1) {
        return;
    }

    for (size_t i = 0; i < balls.size(); ++i) {
        const PoolBall& ball_phys = balls[i];
        RenderObject& ball_renderable = _dynamicRenderables[i];

        glm::vec3 ball_position_3d = glm::vec3(ball_phys.position.x, BALL_RADIUS, ball_phys.position.y);
        glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), ball_position_3d);
        glm::mat4 rotation_matrix = glm::mat4_cast(ball_phys.rotation);
        glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale_vector);
        ball_renderable.transformMatrix = translation_matrix * rotation_matrix * scale_matrix;
    }

    RenderObject& cue_renderable = _dynamicRenderables.back();

    if (!balls[0].is_moving) {
        const PoolBall& cue_ball_phys = balls[0];
        glm::vec3 cue_ball_pos_3d = glm::vec3(cue_ball_phys.position.x, BALL_RADIUS, cue_ball_phys.position.y);

        glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale_vector);
        glm::mat4 base_transform = y_to_z_up_rotation * scale_matrix;

        glm::mat4 gameplay_rotation = glm::rotate(glm::mat4(1.0f), cue.angle, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 translation_to_ball = glm::translate(glm::mat4(1.0f), cue_ball_pos_3d);

        cue_renderable.transformMatrix = translation_to_ball * gameplay_rotation * base_transform;

    } else {
        cue_renderable.transformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f));
    }
}

void VulkanApplication::initVulkan() {
    std::cout << "[INFO] Initializing Vulkan..." << std::endl;
    createInstance();
    std::cout << "       - Instance created." << std::endl;
    setupDebugMessenger();
    std::cout << "       - Debug messenger set up." << std::endl;
    createSurface();
    std::cout << "       - Surface created." << std::endl;
    pickPhysicalDevice();
    std::cout << "       - Physical device picked." << std::endl;
    createLogicalDevice();
    std::cout << "       - Logical device created." << std::endl;
    createSwapChain();
    std::cout << "       - Swap chain created." << std::endl;
    createImageViews();
    std::cout << "       - Image views created." << std::endl;
    createRenderPass();
    std::cout << "       - Render pass created." << std::endl;
    createDescriptorSetLayout();
    std::cout << "       - Descriptor set layout created." << std::endl;
    createGraphicsPipeline();
    std::cout << "       - Graphics pipeline created." << std::endl;
    createCommandPool();
    std::cout << "       - Command pool created." << std::endl;
    createDepthResources();
    std::cout << "       - Depth resources created." << std::endl;
    createFramebuffers();
    std::cout << "       - Framebuffers created." << std::endl;

    _materials["default"] = Material();
    std::cout << "       - Default material created." << std::endl;
    setupPoolTable();
    std::cout << "       - Pool table set up." << std::endl;
    setup_scene();
    std::cout << "       - Scene set up." << std::endl;

    createUniformBuffers();
    std::cout << "       - Uniform buffers created." << std::endl;
    createDescriptorPool();
    std::cout << "       - Descriptor pool created." << std::endl;
    createDescriptorSets();
    std::cout << "       - Descriptor sets created." << std::endl;
    createCommandBuffer();
    std::cout << "       - Command buffer created." << std::endl;
    createSyncObjects();
    std::cout << "       - Sync objects created." << std::endl;
    std::cout << "[INFO] Vulkan initialized successfully." << std::endl;
}
void VulkanApplication::mainLoop(){
    static auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        if (deltaTime > 0.05f) {
            deltaTime = 0.05f;
        }
        
        processInput(deltaTime);
        updatePhysics(deltaTime);
        update_scene(deltaTime);
        
        drawFrame();
    }
    
    vkDeviceWaitIdle(device);
}

void VulkanApplication::createInstance() {
    if(enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available.");
    }
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Renderer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "MarioRender";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    
    auto required_extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    createInfo.ppEnabledExtensionNames = required_extensions.data();
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
    
    if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void VulkanApplication::setupDebugMessenger() {
    if (!enableValidationLayers) return;
    
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);
    
    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void VulkanApplication::createSurface() {
    if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface.");
    }
}

void VulkanApplication::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    
    if(deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support.");
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    
    for(const auto& device : devices) {
        if(isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }
    
    if(physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU.");
    }
}

void VulkanApplication::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    
    createInfo.pEnabledFeatures = &deviceFeatures;
    
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }
    
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void VulkanApplication::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain.");
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void VulkanApplication::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    if (swapChainImages.empty()) {
        throw std::runtime_error("Swap chain images are empty!");
    }

    for(uint32_t i = 0; i < swapChainImages.size(); i++) {
        if (swapChainImages[i] == VK_NULL_HANDLE) {
            throw std::runtime_error("Invalid image handle in swap chain.");
        }

        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void VulkanApplication::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    if(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass.");
    }
}  

void VulkanApplication::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout.");
    }
}

void VulkanApplication::createGraphicsPipeline() {
    auto vertShaderCode = readFile("shaders/vert.spv");
    auto fragShaderCode = readFile("shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule);
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderModule);
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);

    VkPipelineMultisampleStateCreateInfo multisampling = vkinit::multisampling_state_create_info();

    VkPipelineDepthStencilStateCreateInfo depthStencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS);

    VkPipelineColorBlendAttachmentState colorBlendAttachment = vkinit::color_blend_attachment_state();
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(GPUDrawPushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline.");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void VulkanApplication::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
    VkCommandPoolCreateInfo info = vkinit::command_pool_create_info(queueFamilyIndices.graphicsFamily.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    
    if(vkCreateCommandPool(device, &info, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool.");
    }
}

void VulkanApplication::createDepthResources() {
    VkFormat depthFormat = findDepthFormat();
    
    createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanApplication::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());
    
    for(size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            swapChainImageViews[i],
            depthImageView
        };
        
        VkFramebufferCreateInfo info = vkinit::framebuffer_create_info(renderPass, swapChainExtent, attachments.size(), attachments.data());
        
        if(vkCreateFramebuffer(device, &info, nullptr,&swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer");
        }
    }
}


void VulkanApplication::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    
    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
    
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
        vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void VulkanApplication::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool.");
    }
}

void VulkanApplication::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets.");
    }

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorSets[i], &bufferInfo, 0);

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}


void VulkanApplication::createCommandBuffer() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
    
    if(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers.");
    }
}

void VulkanApplication::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(swapChainImages.size());
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    
    VkSemaphoreCreateInfo semaphoreInfo = vkinit::semaphore_create_info(0);
    
    VkFenceCreateInfo fenceInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);  
    
    for(size_t i = 0; i < swapChainImages.size(); i++) {
        if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores;");
        }
    }
    
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
    }
}

void VulkanApplication::drawFrame() {
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    
    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image.");
    }
    
    updateUniformBuffer(currentFrame);
    
    vkResetFences(device, 1, &inFlightFences[currentFrame]);
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
    
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[imageIndex]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame])!= VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer.");
    }
    
    VkSwapchainKHR swapChains[] = {swapChain};
    
    VkPresentInfoKHR presentInfo = vkinit::present_info(1, signalSemaphores, 1, swapChains, &imageIndex);
    
    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
    
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApplication::updateUniformBuffer(uint32_t currentImage) {
    UniformBufferObject ubo{};

    glm::vec3 lookAtTarget = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 cameraPos;
    cameraPos.x = lookAtTarget.x + cameraDistance * cos(cameraPitch) * cos(cameraYaw);
    cameraPos.y = lookAtTarget.y + cameraDistance * sin(cameraPitch);
    cameraPos.z = lookAtTarget.z + cameraDistance * cos(cameraPitch) * sin(cameraYaw);

    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

    ubo.view = glm::lookAt(cameraPos, lookAtTarget, upVector);

    ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f);

    ubo.proj[1][1] *= -1;

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void VulkanApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer.");
    }

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.05f, 0.1f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo = vkinit::renderpass_begin_info(renderPass, swapChainExtent, swapChainFramebuffers[imageIndex]);
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    auto draw_renderables = [&](const std::vector<RenderObject>& renderables) {
        for (const auto& renderable : renderables) {
            auto mesh_it = _meshes.find(renderable.meshName);
            if (mesh_it == _meshes.end()) { continue; }

            const Mesh& mesh = mesh_it->second;
            if (mesh._vertexBuffer == VK_NULL_HANDLE || mesh._indexBuffer == VK_NULL_HANDLE) { continue; }

            VkBuffer vertexBuffers[] = {mesh._vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, mesh._indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            for (const auto& submesh : mesh._subMeshes) {
                auto mat_it = _materials.find(submesh.materialName);
                if (mat_it == _materials.end()) {
                    mat_it = _materials.find("Default");
                }
                const Material& material = mat_it->second;

                GPUDrawPushConstants pushConstants;
                pushConstants.transform = renderable.transformMatrix;
                pushConstants.color = glm::vec4(material.color, 1.0f);

                vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);

                vkCmdDrawIndexed(commandBuffer, submesh.indexCount, 1, submesh.firstIndex, 0, 0);
            }
        }
    };

    draw_renderables(_staticRenderables);
    draw_renderables(_dynamicRenderables);

    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer.");
    }
}

void VulkanApplication::cleanupSwapChain() {
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
    
    for(auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void VulkanApplication::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createDepthResources();
    createFramebuffers();
}

void VulkanApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

bool VulkanApplication::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
    for(const char* layerName : validationLayers) {
        bool layerFound = false;
        
        for(const auto& layerProperties : availableLayers) {
            if(strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        
        if(!layerFound) return false;
    }
    return true;
}

std::vector<const char*> VulkanApplication::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    
    return extensions;
}

bool VulkanApplication::isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);
    
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    
    bool swapChainAdequate = false;
    if(extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
    
    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool VulkanApplication::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.   data());
    
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end()          );
    
    for(const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    
    return requiredExtensions.empty();
}

QueueFamilyIndices VulkanApplication::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        
        if(presentSupport) {
            indices.presentFamily = i;
        }
        
        if (indices.isComplete()) {
            break;
        }
        
        i++;
    }
    
    return indices;
}

SwapChainSupportDetails VulkanApplication::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if(formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }
    
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if(presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }
    
    return details;
}

VkSurfaceFormatKHR VulkanApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    
    return availableFormats[0];
}

VkPresentModeKHR VulkanApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        
        return actualExtent;
    }
}

VkShaderModule VulkanApplication::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    
    VkShaderModule shaderModule;
    if(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module.");
    }
    
    return shaderModule;
}

uint32_t VulkanApplication::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if((typeFilter & (1 << i )) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    throw std::runtime_error("failed to find suitable memory type.");
}

void VulkanApplication::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer.");
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements) ;
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    
    if(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory.");
    }
    
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void VulkanApplication::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    
    endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer VulkanApplication::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo = vkinit::command_buffer_allocate_info(commandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    return commandBuffer;
}

void VulkanApplication::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo = vkinit::submit_info(&commandBuffer);
    
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);
    
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void VulkanApplication::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    
    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition,");
    }
    
    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    
    endSingleTimeCommands(commandBuffer);
}

void VulkanApplication::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};
    
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    
    endSingleTimeCommands(commandBuffer);
}

void VulkanApplication::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo = vkinit::image_create_info(format, usage, width, height, tiling);
    
    if(vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image.");
    }
    
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    
    if(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory.");
    }
    
    vkBindImageMemory(device, image, imageMemory, 0);
}

VkImageView VulkanApplication::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo = vkinit::imageview_create_info(format, image, aspectFlags);
    
    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
    
    return imageView;
}

VkFormat VulkanApplication::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for(VkFormat format : candidates) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
        
        if(tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
            return format;
        } else if(tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format.");
}

VkFormat VulkanApplication::findDepthFormat() {
    return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT); //VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT
}

bool VulkanApplication::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanApplication::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "[VULKAN VALIDATION]: " << pCallbackData->pMessage << std::endl;
    
    return VK_FALSE;
}
