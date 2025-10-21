#pragma once

#include <vulkan/vulkan_core.h>

namespace veekay::graphics {

struct Buffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
	void* mapped_region;

	Buffer(size_t size, void* data, VkBufferUsageFlags usage);
	~Buffer();
};

} // namespace veekay::graphics
