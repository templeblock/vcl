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
#include <vcl/graphics/vulkan/swapchain.h>

// C++ standard library
#include <vector>

// VCL
#include <vcl/core/contract.h>

namespace Vcl { namespace Graphics { namespace Vulkan
{
	template<typename Func>
	Func getInstanceProc(VkInstance inst, const char* name)
	{
		return reinterpret_cast<Func>(vkGetInstanceProcAddr(inst, name));
	}

	template<typename Func>
	Func getDeviceProc(VkDevice dev, const char* name)
	{
		return reinterpret_cast<Func>(vkGetDeviceProcAddr(dev, name));
	}

#define VCL_VK_GET_INSTANCE_PROC(instance, name) name = getInstanceProc<PFN_##name>(instance, #name)
#define VCL_VK_GET_DEVICE_PROC(device, name) name = getDeviceProc<PFN_##name>(device, #name)

	SwapChain::SwapChain(VkInstance instance, VkDevice device, VkSurfaceKHR surface)
	{
		VCL_VK_GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceSurfaceSupportKHR);
		VCL_VK_GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
		VCL_VK_GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceSurfaceFormatsKHR);
		VCL_VK_GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceSurfacePresentModesKHR);
		VCL_VK_GET_DEVICE_PROC(device, vkCreateSwapchainKHR);
		VCL_VK_GET_DEVICE_PROC(device, vkDestroySwapchainKHR);
		VCL_VK_GET_DEVICE_PROC(device, vkGetSwapchainImagesKHR);
		VCL_VK_GET_DEVICE_PROC(device, vkAcquireNextImageKHR);
		VCL_VK_GET_DEVICE_PROC(device, vkQueuePresentKHR);
	}

	SwapChain::~SwapChain()
	{

	}
}}}
