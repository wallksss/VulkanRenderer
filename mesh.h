#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 texCoord;

  static VkVertexInputBindingDescription getBindingDescription();
  
  static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions(); 

  bool operator==(const Vertex& other) const {
    return pos == other.pos && color == other.color && texCoord == other.texCoord;
  }
};
//struct Mesh {
//  std::vector<Vertex> vertices;

//  AllocatedBuffer vertexBuffer;

//  bool load_from_obj(const char* filename);
//}
