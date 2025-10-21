#include <veekay/graphics.hpp>

#include <stdexcept>
#include <limits>
#include <algorithm>

#include <veekay/application.hpp>
namespace veekay::graphics {

Buffer::Buffer(size_t size, void* data, VkBufferUsageFlags usage) {
	VkDevice& device = veekay::app.vk_device;
	VkPhysicalDevice& physical_device = veekay::app.vk_physical_device;

	{
		VkBufferCreateInfo info{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.usage = usage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};

		if (vkCreateBuffer(device, &info, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan buffer");
		}
	}

	{
		VkMemoryRequirements requirements;
		vkGetBufferMemoryRequirements(device, buffer, &requirements);

		VkPhysicalDeviceMemoryProperties properties;
		vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);

		const VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		uint32_t index = std::numeric_limits<uint32_t>::max();
		for (uint32_t i = 0; i < properties.memoryTypeCount; ++i) {
			const VkMemoryType& type = properties.memoryTypes[i];

			if ((requirements.memoryTypeBits & (1 << i)) &&
			    (type.propertyFlags & flags) == flags) {
				index = i;
				break;
			}
		}

		if (index == std::numeric_limits<uint32_t>::max()) {
			throw std::runtime_error("Failed to find required memory type to allocate Vulkan buffer");
		}

		VkMemoryAllocateInfo info{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = requirements.size,
			.memoryTypeIndex = index,
		};

		if (vkAllocateMemory(device, &info, nullptr, &memory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate Vulkan buffer memory");
		}

		if (vkBindBufferMemory(device, buffer, memory, 0) != VK_SUCCESS) {
			throw std::runtime_error("Failed to bind Vulkan buffer memory");
		}

		void* device_data;
		if (vkMapMemory(device, memory, 0, requirements.size, 0, &device_data) != VK_SUCCESS) {
			throw std::runtime_error("Failed to map Vulkan buffer memory");
		}

		std::copy(static_cast<char*>(data), static_cast<char*>(data) + size,
		          static_cast<char*>(device_data));

		vkUnmapMemory(device, memory);
	}
}

Buffer::~Buffer() {
	VkDevice& device = veekay::app.vk_device;

	vkFreeMemory(device, memory, nullptr);
	vkDestroyBuffer(device, buffer, nullptr);
}

} // namespace veekay::graphics
