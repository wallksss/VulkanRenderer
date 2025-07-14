#pragma once

#include "vk_types.h"
#include <array>

namespace vkinit {
	VkCommandPoolCreateInfo command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

	VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags = 0);

  VkFramebufferCreateInfo framebuffer_create_info(VkRenderPass renderPass, VkExtent2D extent, uint32_t attachmentCount, VkImageView* pAttachments); 

	VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);

	VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags = 0);

	VkSubmitInfo submit_info(VkCommandBuffer* cmd);

  VkPresentInfoKHR present_info(uint32_t waitSemaphoreCount, VkSemaphore* pWaitSemaphores, uint32_t swapchainCount, VkSwapchainKHR* pSwapchains, const uint32_t* pImageIndices); 

	VkRenderPassBeginInfo renderpass_begin_info(VkRenderPass renderPass, VkExtent2D windowExtent, VkFramebuffer framebuffer);

	VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule);

  VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info(const VkVertexInputBindingDescription* bindingDescription, VkVertexInputAttributeDescription* attributeDescriptions, uint32_t descriptionCount); 

	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info(VkPrimitiveTopology topology);

	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(VkPolygonMode polygonMode);

	VkPipelineMultisampleStateCreateInfo multisampling_state_create_info();

	VkPipelineColorBlendAttachmentState color_blend_attachment_state();

  VkPipelineLayoutCreateInfo pipeline_layout_create_info(const VkDescriptorSetLayout* descriptorSetLayout);

  VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, uint32_t width, uint32_t height, VkImageTiling tiling);

	VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);

	VkDescriptorSetLayoutBinding descriptorset_layout_binding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding);

	VkWriteDescriptorSet write_descriptor_buffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding);

	VkWriteDescriptorSet write_descriptor_image(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* imageInfo, uint32_t binding);

	VkSamplerCreateInfo sampler_create_info(VkFilter filters, VkSamplerAddressMode samplerAdressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
}
