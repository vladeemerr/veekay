#pragma once

#include <vulkan/vulkan_core.h>

namespace veekay::graphics {

struct Buffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
	void* mapped_region;

	Buffer(size_t size, const void* data,
	       VkBufferUsageFlags usage);
	~Buffer();

	static size_t structureAlignment(size_t struct_size);
};

struct Texture {
	uint32_t width;
	uint32_t height;
	VkFormat format;

	VkImage image;
	VkImageView view;
	VkDeviceMemory memory;

	Buffer* staging;

	Texture(VkCommandBuffer cmd,
	        uint32_t width, uint32_t height,
	        VkFormat format,
	        const void* pixels);
	~Texture();
};

} // namespace veekay::graphics
