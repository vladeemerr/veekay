#pragma once

namespace veekay {

typedef void (*InitFunc)(void);
typedef void (*UpdateFunc)(void);
typedef void (*RenderFunc)(void);
typedef void (*ShutdownFunc)(void);

struct AppInfo {
	InitFunc init;
	UpdateFunc update;
	RenderFunc render;
	ShutdownFunc shutdown;
};

int run(const AppInfo& app_info);

} // namespace veekay
