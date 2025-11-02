#include <cstdint>
#include <climits>

#include <iostream>
#include <vector>

#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <VkBootstrap.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <veekay/veekay.hpp>

namespace {

constexpr uint32_t window_default_width = 1280;
constexpr uint32_t window_default_height = 720;
constexpr char window_title[] = "Veekay";

constexpr uint32_t max_frames_in_flight = 2;

GLFWwindow* window;

VkInstance vk_instance;
VkDebugUtilsMessengerEXT vk_debug_messenger;
VkPhysicalDevice vk_physical_device;
VkDevice vk_device;
VkSurfaceKHR vk_surface;

VkSwapchainKHR vk_swapchain;
VkFormat vk_swapchain_format;
std::vector<VkImage> vk_swapchain_images;
std::vector<VkImageView> vk_swapchain_image_views;

VkQueue vk_graphics_queue;
uint32_t vk_graphics_queue_family;

// NOTE: ImGui rendering objects
VkDescriptorPool imgui_descriptor_pool;
VkRenderPass imgui_render_pass;
VkCommandPool imgui_command_pool;
std::vector<VkCommandBuffer> imgui_command_buffers;
std::vector<VkFramebuffer> imgui_framebuffers;

VkFormat vk_image_depth_format;
VkImage vk_image_depth;
VkDeviceMemory vk_image_depth_memory;
VkImageView vk_image_depth_view;

VkRenderPass vk_render_pass;
std::vector<VkFramebuffer> vk_framebuffers;

std::vector<VkSemaphore> vk_render_semaphores;
std::vector<VkSemaphore> vk_present_semaphores;
std::vector<VkFence> vk_in_flight_fences;
uint32_t vk_current_frame;

VkCommandPool vk_command_pool;
std::vector<VkCommandBuffer> vk_command_buffers;

} // namespace

namespace veekay {

	Application app;

	namespace input {

		void setup(void* const window_ptr);
		void cache();

	} // namespace input

	namespace graphics {

		void init();

	} // namespace graphics

} // namespace veekay

int veekay::run(const veekay::ApplicationInfo& app_info) {
	veekay::app.running = true;
	
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW\n";
		return 1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(window_default_width, window_default_height,
	                          window_title, nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window\n";
		return 1;
	}

	veekay::input::setup(window);

	/* NOTE:
		needed because otherwise on macos everything will be rendered in the top
		corner of the application window
	*/
#if defined(__APPLE__) && defined(__MACH__)
	int framebuffer_width, framebuffer_height;
	glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

	app.window_width = framebuffer_width;
	app.window_height = framebuffer_height;
#else
	app.window_width = window_default_width;
	app.window_height = window_default_height;
#endif

	{ // NOTE: Initialize Vulkan: grab device and create swapchain
		vkb::InstanceBuilder instance_builder;

		auto builder_result = instance_builder.require_api_version(1, 2, 0)
		                                      .request_validation_layers()
		                                      .use_default_debug_messenger()
		                                      .build();
		if (!builder_result) {
			std::cerr << builder_result.error().message() << '\n';
			return 1;
		}

		auto instance = builder_result.value();

		vk_instance = instance.instance;
		vk_debug_messenger = instance.debug_messenger;

		if (glfwCreateWindowSurface(vk_instance, window, nullptr, &vk_surface) != VK_SUCCESS) {
			const char* message;
			glfwGetError(&message);
			std::cerr << message << '\n';
			return 1;
		}

		vkb::PhysicalDeviceSelector physical_device_selector(instance);

		VkPhysicalDeviceFeatures device_features{
			.samplerAnisotropy = true,
		};

		auto selector_result = physical_device_selector.set_surface(vk_surface)
		                                               .set_required_features(device_features)
		                                               .select();
		if (!selector_result) {
			std::cerr << selector_result.error().message() << '\n';
			return 1;
		}

		auto physical_device = selector_result.value();

		{
			vkb::DeviceBuilder device_builder(physical_device);

			auto result = device_builder.build();

			if (!result) {
				std::cerr << result.error().message() << '\n';
				return 1;
			}

			auto device = result.value();

			vk_device = device.device;
			vk_physical_device = device.physical_device;

			auto queue_type = vkb::QueueType::graphics;
			
			vk_graphics_queue = device.get_queue(queue_type).value();
			vk_graphics_queue_family = device.get_queue_index(queue_type).value();
		}

		vkb::SwapchainBuilder swapchain_builder(vk_physical_device, vk_device, vk_surface);

		vk_swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;

		VkSurfaceFormatKHR surface_format{
			.format = vk_swapchain_format,
			.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		};

		auto swapchain_result = swapchain_builder.set_desired_format(surface_format)
		                                         .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		                                         .set_desired_extent(app.window_width, app.window_height)
		                                         .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		                                         .build();

		if (!swapchain_result) {
			std::cerr << swapchain_result.error().message() << '\n';
			return 1;
		}

		auto swapchain = swapchain_result.value();

		vk_swapchain = swapchain.swapchain;
		vk_swapchain_images = swapchain.get_images().value();
		vk_swapchain_image_views = swapchain.get_image_views().value();

		veekay::app.vk_device = vk_device;
		veekay::app.vk_physical_device = vk_physical_device;
	}

	graphics::init();

	{ // NOTE: ImGui initialization
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForVulkan(window, true);

		{
			VkDescriptorPoolSize size = {
				.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE,
			};

			VkDescriptorPoolCreateInfo info = {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
				.maxSets = size.descriptorCount,
				.poolSizeCount = 1,
				.pPoolSizes = &size,
			};

			if (vkCreateDescriptorPool(vk_device, &info, 0, &imgui_descriptor_pool) != VK_SUCCESS) {
				std::cerr << "Failed to create Vulkan descriptor pool for ImGui\n";
				return 1;
			}
		}

		{
			VkAttachmentDescription attachment{
				.format = vk_swapchain_format,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			};

			VkAttachmentReference ref{
				.attachment = 0,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			};

			VkSubpassDescription subpass{
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments = &ref,
			};

			VkSubpassDependency dependency{
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			};

			VkRenderPassCreateInfo info{
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
				.attachmentCount = 1,
				.pAttachments = &attachment,
				.subpassCount = 1,
				.pSubpasses = &subpass,
				.dependencyCount = 1,
				.pDependencies = &dependency,
			};

			if (vkCreateRenderPass(vk_device, &info, nullptr, &imgui_render_pass) != VK_SUCCESS) {
				std::cerr << "Failed to create ImGui Vulkan render pass\n";
				return 1;
			}
		}

		{
			VkFramebufferCreateInfo info{
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = imgui_render_pass,
				.attachmentCount = 1,
				.width = app.window_width,
				.height = app.window_height,
				.layers = 1,
			};

			const size_t count = vk_swapchain_images.size();

			imgui_framebuffers.resize(count);

			for (size_t i = 0; i < count; ++i) {
				info.pAttachments = &vk_swapchain_image_views[i];
				if (vkCreateFramebuffer(vk_device, &info, nullptr, &imgui_framebuffers[i]) != VK_SUCCESS) {
					std::cerr << "Failed to create Vulkan framebuffer " << i << '\n';
					return 1;
				}
			}
		}

		{
			size_t count = imgui_framebuffers.size();

			imgui_command_buffers.resize(count);

			{
				VkCommandPoolCreateInfo info{
					.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
					.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
					.queueFamilyIndex = vk_graphics_queue_family,
				};

				if (vkCreateCommandPool(vk_device, &info, nullptr, &imgui_command_pool) != VK_SUCCESS) {
					std::cerr << "Failed to create ImGui Vulkan command pool\n";
					return 1;
				}
			}

			{
				VkCommandBufferAllocateInfo info{
					.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
					.commandPool = imgui_command_pool,
					.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
					.commandBufferCount = static_cast<uint32_t>(imgui_command_buffers.size()),
				};

				if (vkAllocateCommandBuffers(vk_device, &info, imgui_command_buffers.data()) != VK_SUCCESS) {
					std::cerr << "Failed to allocate ImGui Vulkan command buffers\n";
					return 1;
				}
			}
		}

		ImGui_ImplVulkan_InitInfo info{
			.Instance = vk_instance,
			.PhysicalDevice = vk_physical_device,
			.Device = vk_device,
			.QueueFamily = vk_graphics_queue_family,
			.Queue = vk_graphics_queue,
			.DescriptorPool = imgui_descriptor_pool,
			.MinImageCount = static_cast<uint32_t>(vk_swapchain_images.size()),
			.ImageCount = static_cast<uint32_t>(vk_swapchain_images.size()),
			.RenderPass = imgui_render_pass,
		};

		ImGui_ImplVulkan_Init(&info);
	}

	{
		VkFormat candidates[] = {
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
		};

		vk_image_depth_format = VK_FORMAT_UNDEFINED;

		for (const auto& f : candidates) {
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(vk_physical_device, f, &properties);

			if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				vk_image_depth_format = f;
				break;
			}
		}
	}

	{ // NOTE: Create depth buffer
		VkImageCreateInfo info{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = vk_image_depth_format,
			.extent = {app.window_width, app.window_height, 1},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		};

		if (vkCreateImage(vk_device, &info, nullptr, &vk_image_depth) != VK_SUCCESS) {
			std::cerr << "Failed to create Vulkan depth image\n";
			return 1;
		}
	}

	{ // NOTE: Allocate depth buffer memory
		VkMemoryRequirements requirements;
		vkGetImageMemoryRequirements(vk_device, vk_image_depth, &requirements);

		VkPhysicalDeviceMemoryProperties properties;
		vkGetPhysicalDeviceMemoryProperties(vk_physical_device, &properties);

		uint32_t index = UINT_MAX;
		for (uint32_t i = 0; i < properties.memoryTypeCount; ++i) {
			const VkMemoryType& type = properties.memoryTypes[i];

			if ((requirements.memoryTypeBits & (1 << i)) &&
			    (type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
				index = i;
				break;
			}
		}

		if (index == UINT_MAX) {
			std::cerr << "Failed to find required memory type for Vulkan depth image\n";
			return 1;
		}

		VkMemoryAllocateInfo info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = requirements.size,
			.memoryTypeIndex = index,
		};

		if (vkAllocateMemory(vk_device, &info, nullptr, &vk_image_depth_memory) != VK_SUCCESS) {
			std::cerr << "Failed to allocate memory for Vulkan depth image\n";
			return 1;
		}

		if (vkBindImageMemory(vk_device, vk_image_depth, vk_image_depth_memory, 0) != VK_SUCCESS) {
			std::cerr << "Failed to bind Vulkan depth image with device memory\n";
			return 1;
		}
	}

	{ // NOTE: Create depth buffer view object
		VkImageViewCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = vk_image_depth,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = vk_image_depth_format,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		if (vkCreateImageView(vk_device, &info, nullptr, &vk_image_depth_view) != VK_SUCCESS) {
			std::cerr << "Failed to create Vulkan depth image view\n";
			return 1;
		}
	}

	{ // NOTE: Create render pass
		VkAttachmentDescription color_attachment{
			.format = vk_swapchain_format,

			.samples = VK_SAMPLE_COUNT_1_BIT,

			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,

			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,

			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		};

		VkAttachmentDescription depth_attachment{
			.format = vk_image_depth_format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};

		VkAttachmentReference color_ref{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};

		VkAttachmentReference depth_ref{
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};

		VkSubpassDescription subpass{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &color_ref,
			.pDepthStencilAttachment = &depth_ref,
		};

		VkAttachmentDescription attachments[] = {color_attachment, depth_attachment};

		VkSubpassDependency dependency{
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
			                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
			                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			                 VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
		};

		VkRenderPassCreateInfo info{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,

			.attachmentCount = 2,
			.pAttachments = attachments,

			.subpassCount = 1,
			.pSubpasses = &subpass,

			.dependencyCount = 1,
			.pDependencies = &dependency,
		};

		if (vkCreateRenderPass(vk_device, &info, nullptr, &vk_render_pass) != VK_SUCCESS) {
			std::cerr << "Failed to create render pass\n";
			return 1;
		}

		veekay::app.vk_render_pass = vk_render_pass;
	}

	{ // NOTE: Create framebuffer objects from swapchain images
		VkImageView attachments[] = {VK_NULL_HANDLE, vk_image_depth_view};

		VkFramebufferCreateInfo info{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,

			.renderPass = vk_render_pass,

			.attachmentCount = 2,
			.pAttachments = attachments,

			.width = app.window_width,
			.height = app.window_height,
			.layers = 1,
		};

		const size_t count = vk_swapchain_images.size();

		vk_framebuffers.resize(count);

		for (size_t i = 0; i < count; ++i) {
			attachments[0] = vk_swapchain_image_views[i];
			if (vkCreateFramebuffer(vk_device, &info, nullptr, &vk_framebuffers[i]) != VK_SUCCESS) {
				std::cerr << "Failed to create Vulkan framebuffer " << i << '\n';
				return 1;
			}
		}
	}

	{ // NOTE: Create sync primitives
		VkFenceCreateInfo fence_info{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};

		VkSemaphoreCreateInfo sem_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};

		vk_present_semaphores.resize(vk_swapchain_images.size());

		for (size_t i = 0, e = vk_swapchain_images.size(); i != e; ++i) {
			vkCreateSemaphore(vk_device, &sem_info, nullptr, &vk_present_semaphores[i]);
		}

		vk_render_semaphores.resize(max_frames_in_flight);
		vk_in_flight_fences.resize(max_frames_in_flight);

		for (uint32_t i = 0; i < max_frames_in_flight; ++i) {
			vkCreateSemaphore(vk_device, &sem_info, nullptr, &vk_render_semaphores[i]);
			vkCreateFence(vk_device, &fence_info, nullptr, &vk_in_flight_fences[i]);
		}
	}

	{ // NOTE: Create command pool from graphics queue
		VkCommandPoolCreateInfo info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = vk_graphics_queue_family,
		};

		if (vkCreateCommandPool(vk_device, &info, nullptr, &vk_command_pool) != VK_SUCCESS) {
			std::cerr << "Failed to create Vulkan command pool\n";
			return 1;
		}
	}

	{ // NOTE: Allocate command buffers
		vk_command_buffers.resize(vk_framebuffers.size());
		
		VkCommandBufferAllocateInfo info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = vk_command_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = static_cast<uint32_t>(vk_command_buffers.size()),
		};

		if (vkAllocateCommandBuffers(vk_device, &info, vk_command_buffers.data()) != VK_SUCCESS) {
			std::cerr << "Failed to allocate Vulkan command buffers\n";
			return 1;
		}
	}

	VkCommandBuffer onetime_command_buffer; {
		VkCommandBufferAllocateInfo info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = vk_command_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		if (vkAllocateCommandBuffers(vk_device, &info, &onetime_command_buffer) != VK_SUCCESS) {
			std::cerr << "Failed to allocate Vulkan one-time command buffers\n";
			return 1;
		}
	}

	{
		VkCommandBufferBeginInfo info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		};

		vkBeginCommandBuffer(onetime_command_buffer, &info);
	}

	app_info.init(onetime_command_buffer);

	{
		vkEndCommandBuffer(onetime_command_buffer);

		VkSubmitInfo info{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &onetime_command_buffer,
		};

		vkQueueSubmit(vk_graphics_queue, 1, &info, VK_NULL_HANDLE);
		vkQueueWaitIdle(vk_graphics_queue);

		vkFreeCommandBuffers(vk_device, vk_command_pool, 1, &onetime_command_buffer);
	}

	while (veekay::app.running && !glfwWindowShouldClose(window)) {
		veekay::input::cache();
		
		glfwPollEvents();
		double time = glfwGetTime();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		app_info.update(time);

		ImGui::Render();

		// NOTE: Wait until the previous frame finishes
		vkWaitForFences(vk_device, 1, &vk_in_flight_fences[vk_current_frame], true, UINT64_MAX);
		vkResetFences(vk_device, 1, &vk_in_flight_fences[vk_current_frame]);

		// NOTE: Get current swapchain framebuffer index
		uint32_t swapchain_image_index = 0;
		vkAcquireNextImageKHR(vk_device, vk_swapchain, UINT64_MAX,
		                      vk_render_semaphores[vk_current_frame],
		                      nullptr, &swapchain_image_index);

		VkCommandBuffer cmd = vk_command_buffers[swapchain_image_index];

		app_info.render(cmd, vk_framebuffers[swapchain_image_index]);

		VkCommandBuffer imgui_cmd = imgui_command_buffers[swapchain_image_index];
		{ // NOTE: Draw ImGui
			vkResetCommandBuffer(imgui_cmd, 0);

			{
				VkCommandBufferBeginInfo info{
					.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
					.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
				};

				vkBeginCommandBuffer(imgui_cmd, &info);
			}

			{
				VkRenderPassBeginInfo info{
					.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.renderPass = imgui_render_pass,
					.framebuffer = imgui_framebuffers[swapchain_image_index],
					.renderArea = {
						.extent = {app.window_width, app.window_height},
					},
				};

				vkCmdBeginRenderPass(imgui_cmd, &info, VK_SUBPASS_CONTENTS_INLINE);
			}

			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imgui_cmd);

			vkCmdEndRenderPass(imgui_cmd);
			vkEndCommandBuffer(imgui_cmd);
		}

		{ // NOTE: Submit commands to graphics queue
			VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			VkCommandBuffer buffers[] = { cmd, imgui_cmd };

			VkSubmitInfo info{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &vk_render_semaphores[vk_current_frame],
				.pWaitDstStageMask = &wait_stage,
				.commandBufferCount = 2,
				.pCommandBuffers = buffers,
				.signalSemaphoreCount = 1,
				.pSignalSemaphores = &vk_present_semaphores[swapchain_image_index],
			};

			vkQueueSubmit(vk_graphics_queue, 1, &info, vk_in_flight_fences[vk_current_frame]);
		}

		{ // NOTE: Present renderer frame
			VkPresentInfoKHR info{
				.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &vk_present_semaphores[swapchain_image_index],
				.swapchainCount = 1,
				.pSwapchains = &vk_swapchain,
				.pImageIndices = &swapchain_image_index,
			};

			vkQueuePresentKHR(vk_graphics_queue, &info);

			vk_current_frame = (vk_current_frame + 1) % max_frames_in_flight;
		}
	}

	vkDeviceWaitIdle(vk_device);

	app_info.shutdown();

	vkDestroyCommandPool(vk_device, vk_command_pool, nullptr);

	for (size_t i = 0, e = vk_swapchain_images.size(); i != e; ++i) {
		vkDestroySemaphore(vk_device, vk_present_semaphores[i], nullptr);
	}

	for (size_t i = 0; i < max_frames_in_flight; ++i) {
		vkDestroySemaphore(vk_device, vk_render_semaphores[i], nullptr);
		vkDestroyFence(vk_device, vk_in_flight_fences[i], nullptr);
	}
	
	vkDestroyRenderPass(vk_device, vk_render_pass, nullptr);

	vkDestroyImageView(vk_device, vk_image_depth_view, nullptr);
	vkFreeMemory(vk_device, vk_image_depth_memory, nullptr);
	vkDestroyImage(vk_device, vk_image_depth, nullptr);

	vkDestroyCommandPool(vk_device, imgui_command_pool, nullptr);
	vkDestroyRenderPass(vk_device, imgui_render_pass, nullptr);

	for (size_t i = 0, e = vk_framebuffers.size(); i != e; ++i) {
		vkDestroyFramebuffer(vk_device, vk_framebuffers[i], nullptr);
		vkDestroyFramebuffer(vk_device, imgui_framebuffers[i], nullptr);
		vkDestroyImageView(vk_device, vk_swapchain_image_views[i], nullptr);
	}

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	vkDestroyDescriptorPool(vk_device, imgui_descriptor_pool, nullptr);
	
	vkDestroySwapchainKHR(vk_device, vk_swapchain, nullptr);
	vkDestroyDevice(vk_device, nullptr);
	vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);
	vkb::destroy_debug_utils_messenger(vk_instance, vk_debug_messenger);
	vkDestroyInstance(vk_instance, nullptr);

	glfwDestroyWindow(window);
	glfwTerminate();
	
	return 0;
}
