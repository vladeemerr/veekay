#pragma once

#include <vulkan/vulkan_core.h>

namespace veekay {

typedef void (*InitFunc)(VkCommandBuffer);
typedef void (*ShutdownFunc)();
typedef void (*UpdateFunc)(double time);
typedef void (*RenderFunc)(VkCommandBuffer, VkFramebuffer);

struct Application {
	uint32_t window_width;
	uint32_t window_height;

	VkDevice vk_device;
	VkPhysicalDevice vk_physical_device;
	VkRenderPass vk_render_pass;

	bool running;
};

struct ApplicationInfo {
	InitFunc init;
	ShutdownFunc shutdown;
	UpdateFunc update;
	RenderFunc render;
};

extern Application app;

int run(const ApplicationInfo& app_info);

} // namespace veekay
