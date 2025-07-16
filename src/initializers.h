#pragma once

#include "vk_types.h"
#include <array>

namespace vkinit {
	// Creates a VkCommandPoolCreateInfo structure.
	VkCommandPoolCreateInfo command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

	// Creates a VkCommandBufferAllocateInfo structure.
	VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	// Creates a VkCommandBufferBeginInfo structure.
	VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags = 0);

	// Creates a VkFramebufferCreateInfo structure.
	VkFramebufferCreateInfo framebuffer_create_info(VkRenderPass renderPass, VkExtent2D extent, uint32_t attachmentCount, VkImageView* pAttachments);

	// Creates a VkFenceCreateInfo structure.
	VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);

	// Creates a VkSemaphoreCreateInfo structure.
	VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags = 0);

	// Creates a VkSubmitInfo structure.
	VkSubmitInfo submit_info(VkCommandBuffer* cmd);

	// Creates a VkPresentInfoKHR structure.
	VkPresentInfoKHR present_info(uint32_t waitSemaphoreCount, VkSemaphore* pWaitSemaphores, uint32_t swapchainCount, VkSwapchainKHR* pSwapchains, const uint32_t* pImageIndices);

	// Creates a VkRenderPassBeginInfo structure.
	VkRenderPassBeginInfo renderpass_begin_info(VkRenderPass renderPass, VkExtent2D windowExtent, VkFramebuffer framebuffer);

	// Creates a VkPipelineShaderStageCreateInfo structure.
	VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule);

	// Creates a VkPipelineVertexInputStateCreateInfo structure.
	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info(const VkVertexInputBindingDescription* bindingDescription, VkVertexInputAttributeDescription* attributeDescriptions, uint32_t descriptionCount);

	// Creates a VkPipelineInputAssemblyStateCreateInfo structure.
	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info(VkPrimitiveTopology topology);

	// Creates a VkPipelineRasterizationStateCreateInfo structure.
	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(VkPolygonMode polygonMode);

	// Creates a VkPipelineMultisampleStateCreateInfo structure.
	VkPipelineMultisampleStateCreateInfo multisampling_state_create_info();

	// Creates a VkPipelineColorBlendAttachmentState structure.
	VkPipelineColorBlendAttachmentState color_blend_attachment_state();

	// Creates a VkPipelineLayoutCreateInfo structure.
	VkPipelineLayoutCreateInfo pipeline_layout_create_info(const VkDescriptorSetLayout* descriptorSetLayout);

	// Creates a VkImageCreateInfo structure.
	VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, uint32_t width, uint32_t height, VkImageTiling tiling);

	// Creates a VkImageViewCreateInfo structure.
	VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

	// Creates a VkPipelineDepthStencilStateCreateInfo structure.
	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);

	// Creates a VkDescriptorSetLayoutBinding structure.
	VkDescriptorSetLayoutBinding descriptorset_layout_binding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding);

	// Creates a VkWriteDescriptorSet structure for a buffer.
	VkWriteDescriptorSet write_descriptor_buffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding);

	// Creates a VkWriteDescriptorSet structure for an image.
	VkWriteDescriptorSet write_descriptor_image(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* imageInfo, uint32_t binding);

	// Creates a VkSamplerCreateInfo structure.
	VkSamplerCreateInfo sampler_create_info(VkFilter filters, VkSamplerAddressMode samplerAdressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
}