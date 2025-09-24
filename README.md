# 🌋 Veekay

## Getting started

You need C++ compiler, Vulkan SDK and CMake installed before you can build this project.

This project uses C++20 standard and thus requires either of those compilers:
- GCC 10.X
- Clang 10
- Microsoft Visual Studio 2019

Veekay is officially tested on *Windows* and *GNU/Linux platforms*, no *macOS* support yet.
If you have a working macOS solution of this code, consider submitting a PR so others
can build this example code without a hassle!

<ins>**1. Downloading the repository**</ins>

Start by cloning the repository with `git clone --depth 1 https://github.com/vladeemerr/veekay`

This repository does not contain any submodules, it utilizes CMake's `FetchContent` feature instead.

<ins>**2. Configuring the project**</ins>

Run either one of the CMake lines to download dependencies and configure the project:

```bash
cmake --preset debug      # for GNU/Linux (GCC/Clang)
cmake --preset msvc-debug # for Windows (Visual Studio 2019)
```

If you wish to build in `release` mode, change `debug` to `release`.

If changes are made (added/removed files), or if you want to regenerate project files, rerun the command above.

<ins>**3. Building**</ins>

To build the project, use the line below. You are most likely using `debug` preset, so
the directory that will eventually contain your build files is named `build-debug`.

Likewise for `release` that directory will be named `build-release`

Run one those commands, depending on which preset you chose:

```bash
cmake --build build-debug --parallel # for debug
cmake --build build-release --parallel # for release
```

## Project structure

### Overview

Veekay consists of two parts: library and application

* `source` directory contains library code
* `testbed` directory contains application code

Library code contains most of the boilerplate for GLFW, Vulkan and ImGui initialization.
Veekay library also takes care of managing swapchain and giving you relevant
`VkCommandBuffer` and `VkFramebuffer` for you to submit/render to.

However, the majority of your work will happen in `testbed`.
This is where you will write most of your application code.
It is already linked with Veekay library and contains its own `CMakeLists.txt`
build recipe for you to modify.

### Application code

`veekay.hpp` header exposes library functionality through a set of callbacks
(`init`, `shutdown`, `update`, `render`) and global variable `app`.

Look for `testbed/main.cpp`, this is where you start.

`veekay::Application` contains important data like window size, `VkDevice`,
`VkPhysicalDevice` and `VkRenderPass` (associated with a swapchain).

So, say you want to create a `VkBuffer`. This is how you would do it:

```c++
// You fill info struct before calling vkCreateXXX
vkCreateBuffer(veekay::app.vk_device, &info, nullptr, &vertex_buffer);
```

Notice the `veekay::app.vk_device`, `veekay` is a library namespace,
`app` is a global state variable provided by Veekay and `vk_device` is
a `VkDevice` contained in `app` variable.

### Running

`build-xxx/testbed` will contain the executable after successful build

**Make sure your working directory is set to the project root!**
Project root is where this README file resides. Otherwise, the
code responsible for loading shaders from files will fail, because relative paths are used.

### Compiling shaders

`testbed/CMakeLists.txt` has build recipe for compiling shader files
along with an application. Look for a comment in this file to see
how to compile your shaders.
