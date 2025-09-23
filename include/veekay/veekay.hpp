#pragma once

#include <vulkan/vulkan_core.h>

namespace veekay {

typedef void (*InitFunc)();
typedef void (*UpdateFunc)(double time);
typedef void (*RenderFunc)();
typedef void (*ShutdownFunc)();

struct Application {
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
