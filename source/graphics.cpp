#include <veekay/graphics.hpp>

#include <stdexcept>
#include <limits>
#include <algorithm>

#include <veekay/application.hpp>
namespace veekay::graphics {

Buffer::Buffer(size_t size, const void* data,
               VkBufferUsageFlags usage) {
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

		if (vkMapMemory(device, memory, 0, requirements.size, 0, &mapped_region) != VK_SUCCESS) {
			throw std::runtime_error("Failed to map Vulkan buffer memory");
		}

		if (data != nullptr) {
			std::copy(static_cast<const char*>(data),
			          static_cast<const char*>(data) + size,
			          static_cast<char*>(mapped_region));
		}
	}
}

Buffer::~Buffer() {
	VkDevice& device = veekay::app.vk_device;

	vkFreeMemory(device, memory, nullptr);
	vkDestroyBuffer(device, buffer, nullptr);
}

Texture::Texture(VkCommandBuffer cmd,
                 uint32_t width, uint32_t height,
                 VkFormat format,
                 const void* pixels)
: width{width}, height{height}, format{format} {
	VkDevice& device = veekay::app.vk_device;
	VkPhysicalDevice& physical_device = veekay::app.vk_physical_device;

	{
		VkImageCreateInfo info{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = format,
			.extent = {
				.width = width,
				.height = height,
				.depth = 1,
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};

		if (vkCreateImage(device, &info, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan image");
		}
	}

	{
		VkMemoryRequirements requirements;
		vkGetImageMemoryRequirements(device, image, &requirements);

		VkPhysicalDeviceMemoryProperties properties;
		vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);

		const VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

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
			throw std::runtime_error("Failed to allocate Vulkan image memory");
		}

		if (vkBindImageMemory(device, image, memory, 0) != VK_SUCCESS) {
			throw std::runtime_error("Failed to bind Vulkan image memory");
		}
	}

	VkImageSubresourceRange range{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	{
		VkImageViewCreateInfo info{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = format,
			.subresourceRange = range,
		};

		if (vkCreateImageView(device, &info, nullptr, &view) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan image view");
		}
	}

	staging = new Buffer(width * height * sizeof(uint32_t),
	                     pixels,
	                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	VkImageMemoryBarrier undef_to_dst{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = range,
	};

	vkCmdPipelineBarrier(cmd,
	                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	                     VK_PIPELINE_STAGE_TRANSFER_BIT,
	                     0,
	                     0, nullptr,
	                     0, nullptr,
	                     1, &undef_to_dst);

	VkBufferImageCopy copy_info{
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,

		.imageSubresource = {
			.aspectMask = range.aspectMask,
			.mipLevel = range.baseMipLevel,
			.baseArrayLayer = range.baseArrayLayer,
			.layerCount = range.levelCount,
		},

		.imageOffset = {0, 0, 0},
		.imageExtent = {width, height, 1},
	};

	vkCmdCopyBufferToImage(cmd, staging->buffer, image,
	                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                       1, &copy_info);

	VkImageMemoryBarrier dst_to_sample{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = range,
	};

	vkCmdPipelineBarrier(cmd,
	                     VK_PIPELINE_STAGE_TRANSFER_BIT,
	                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	                     0,
	                     0, nullptr,
	                     0, nullptr,
	                     1, &dst_to_sample);
}

Texture::~Texture() {
	VkDevice& device = veekay::app.vk_device;

	delete staging;

	vkFreeMemory(device, memory, nullptr);
	vkDestroyImageView(device, view, nullptr);
	vkDestroyImage(device, image, nullptr);
}

} // namespace veekay::graphics
