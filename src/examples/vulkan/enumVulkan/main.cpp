/*
 * This file is part of the Visual Computing Library (VCL) release under the
 * MIT license.
 *
 * Copyright (c) 2016 Basil Fierz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// VCL configuration
#include <vcl/config/global.h>

// C++ standard library
#include <iostream>
#include <vector>

// Vulkan API
#include <vulkan/vulkan.h>

// GLFW
#include <GLFW/glfw3.h>

// VCL
#include <vcl/graphics/vulkan/commands.h>
#include <vcl/graphics/vulkan/platform.h>
#include <vcl/graphics/vulkan/swapchain.h>

class GlfwInstance
{
public:
	GlfwInstance()
	{
		glfwSetErrorCallback(errorCallback);
		_isInitialized = glfwInit() != 0 ? true : false;
		if (!_isInitialized)
			throw std::runtime_error{ "Unexpected error when initializing GLFW" };

		// Since we are using vulkan we do not need any predefined client API
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Check Vulkan support
		_vulkanSupport = glfwVulkanSupported() != 0 ? true : false;
		
		unsigned int nr_exts = 0;
		auto exts = glfwGetRequiredInstanceExtensions(&nr_exts);
		_vulkanExtensions = { exts, nr_exts };
	}

	~GlfwInstance()
	{
		if (_isInitialized)
			glfwTerminate();
	}

public:
	//! \returns true if vulkan is supported
	bool isVulkanSupported() const { return _vulkanSupport; }

	//! \returns the required vulkan extensions
	gsl::span<const char*> vulkanExtensions() const { return _vulkanExtensions; }

private:
	static void errorCallback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}

private:
	bool _isInitialized{ false };

	//! Query if Vulkan is supported
	bool _vulkanSupport{ false };

	//! Required Vulkan extensions
	gsl::span<const char*> _vulkanExtensions;
};


int main(int argc, char* argv[])
{
	using namespace Vcl::Graphics::Vulkan;

	GlfwInstance glfw;
	if (!glfw.isVulkanSupported())
	{
		std::cerr << "Vulkan is not supported" << std::endl;
		return 1;
	}

	// Vulkan extension
	std::array<const char*, 1> context_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	// Initialize the Vulkan platform
	auto platform = std::make_unique<Platform>(glfw.vulkanExtensions());
	auto device = platform->device(0);
	auto context = device.createContext(context_extensions);
	CommandQueue queue{ context->queue(0) };
	CommandPool pool{ *context, 0};
	CommandBuffer setup_buffer{ *context, pool };

	// Create a window
	auto window = glfwCreateWindow(1280, 720, "Vulkan Demo", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Cannot create a window in which to draw!" << std::endl;
		return 1;
	}
	
	// Begin: Setup surface and swap-chain

	// Create a WSI surface for the window
	VkSurfaceKHR surface_ctx;
	glfwCreateWindowSurface(*platform, window, nullptr, &surface_ctx);

	// Create a swap-chain
	Surface surface{ *platform, device, surface_ctx };

	SwapChainDescription desc;
	desc.Surface = surface;
	desc.NumberOfImages = 1;
	desc.ColourFormat = VK_FORMAT_B8G8R8A8_UNORM;
	desc.ColourSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	desc.Width = 1280;
	desc.Height = 720;
	desc.PresentMode = VK_PRESENT_MODE_FIFO_KHR;
	desc.PreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

	setup_buffer.begin();
	SwapChain swapchain{ *context, setup_buffer, desc };
	setup_buffer.end();
	queue.submit(setup_buffer);
	queue.waitIdle();
	
	// End: Setup surface and swap-chain

	// Enter the event-loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	return 0;
}
