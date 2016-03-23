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
#include <vcl/graphics/vulkan/tools.h>

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

	Surface::Surface(VkInstance instance, VkPhysicalDevice device, VkSurfaceKHR surface)
	: _instance(instance)
	, _device(device)
	, _surface(surface)
	{
		VkResult res;

		VCL_VK_GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceSurfaceSupportKHR);
		VCL_VK_GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
		VCL_VK_GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceSurfaceFormatsKHR);
		VCL_VK_GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceSurfacePresentModesKHR);

		// Get list of supported surface formats
		uint32_t nr_formats;
		res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &nr_formats, nullptr);

		std::vector<VkSurfaceFormatKHR> surface_formats(nr_formats);
		res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &nr_formats, surface_formats.data());

		// If the only format is VK_FORMAT_UNDEFINED
		//if ((nr_formats == 1) && (surface_formats[0].format == VK_FORMAT_UNDEFINED))
		//{
		//	_colourFormat = VK_FORMAT_B8G8R8A8_UNORM;
		//	_colourSpace = surface_formats[0].colorSpace;
		//}
		//else
		//{
		//}

		// Query surface properties and formats
		VkSurfaceCapabilitiesKHR surface_caps;
		res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &surface_caps);
		Check(res == VK_SUCCESS, "Surface capabilities are queried.");

		// Query available present modes
		uint32_t nr_present_modes;
		res = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &nr_present_modes, nullptr);
		Check(res == VK_SUCCESS, "Number of present modes are queried.");
		
		std::vector<VkPresentModeKHR> present_modes(nr_present_modes);
		res = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &nr_present_modes, present_modes.data());
		Check(res == VK_SUCCESS, "Present modes are queried.");
	}

	Surface::~Surface()
	{
		if (_surface)
			vkDestroySurfaceKHR(_instance, _surface, nullptr);
	}

	SwapChain::SwapChain(VkDevice context, VkCommandBuffer cmd_buffer, const SwapChainDescription& desc)
	: _context(context)
	, _desc(desc)
	{
		VkResult res;

		VCL_VK_GET_DEVICE_PROC(context, vkCreateSwapchainKHR);
		VCL_VK_GET_DEVICE_PROC(context, vkDestroySwapchainKHR);
		VCL_VK_GET_DEVICE_PROC(context, vkGetSwapchainImagesKHR);
		VCL_VK_GET_DEVICE_PROC(context, vkAcquireNextImageKHR);
		VCL_VK_GET_DEVICE_PROC(context, vkQueuePresentKHR);

		VkSwapchainCreateInfoKHR sc_create_info;
		sc_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		sc_create_info.pNext = nullptr;
		sc_create_info.surface = desc.Surface;
		sc_create_info.minImageCount = desc.NumberOfImages;
		sc_create_info.imageFormat = desc.ColourFormat;
		sc_create_info.imageColorSpace = desc.ColourSpace;
		sc_create_info.imageExtent = { desc.Width, desc.Height };
		sc_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		sc_create_info.preTransform = (VkSurfaceTransformFlagBitsKHR) desc.PreTransform;
		sc_create_info.imageArrayLayers = 1;
		sc_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		sc_create_info.queueFamilyIndexCount = 0;
		sc_create_info.pQueueFamilyIndices = NULL;
		sc_create_info.presentMode = desc.PresentMode;
		sc_create_info.oldSwapchain = nullptr;//oldSwapchain;
		sc_create_info.clipped = true;
		sc_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		res = vkCreateSwapchainKHR(context, &sc_create_info, nullptr, &_swapchain);
		Check(res == VK_SUCCESS, "Swap chain was created.");

		uint32_t nr_images = 0;
		res = vkGetSwapchainImagesKHR(context, _swapchain, &nr_images, nullptr);
		Check(res == VK_SUCCESS, "Number of images can be queried.");

		// Ask the implementation for the swap-chain images
		_images.resize(nr_images);
		res = vkGetSwapchainImagesKHR(context, _swapchain, &nr_images, _images.data());
		Check(res == VK_SUCCESS, "Images can be accessed.");

		// Create the image views for the swap-chain images
		_views.resize(nr_images);
		for (uint32_t i = 0; i < nr_images; i++)
		{
			VkImageViewCreateInfo colorAttachmentView = {};
			colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorAttachmentView.pNext = nullptr;
			colorAttachmentView.format = desc.ColourFormat;
			colorAttachmentView.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorAttachmentView.subresourceRange.baseMipLevel = 0;
			colorAttachmentView.subresourceRange.levelCount = 1;
			colorAttachmentView.subresourceRange.baseArrayLayer = 0;
			colorAttachmentView.subresourceRange.layerCount = 1;
			colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorAttachmentView.flags = 0;

			// Transform images from initial (undefined) to present layout
			setImageLayout
			(
				cmd_buffer,
				_images[i],
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			);

			colorAttachmentView.image = _images[i];

			res = vkCreateImageView(_context, &colorAttachmentView, nullptr, &_views[i]);
			Check(res == VK_SUCCESS, "Image view was created.", "Image index: %d", i);
		}
	}

	SwapChain::~SwapChain()
	{
		for (uint32_t i = 0; i < _images.size(); i++)
			vkDestroyImageView(_context, _views[i], nullptr);

		if (_swapchain)
			vkDestroySwapchainKHR(_context, _swapchain, nullptr);
	}

	VkResult SwapChain::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* currentBuffer)
	{
		return vkAcquireNextImageKHR(_context, _swapchain, UINT64_MAX, presentCompleteSemaphore, (VkFence)nullptr, currentBuffer);
	}

	VkResult SwapChain::queuePresent(VkQueue queue, uint32_t currentBuffer)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &_swapchain;
		presentInfo.pImageIndices = &currentBuffer;

		return vkQueuePresentKHR(queue, &presentInfo);
	}

	VkResult SwapChain::queuePresent(VkQueue queue, uint32_t currentBuffer, VkSemaphore waitSemaphore)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &_swapchain;
		presentInfo.pImageIndices = &currentBuffer;

		if (waitSemaphore != VK_NULL_HANDLE)
		{
			presentInfo.pWaitSemaphores = &waitSemaphore;
			presentInfo.waitSemaphoreCount = 1;
		}
		return vkQueuePresentKHR(queue, &presentInfo);
	}
}}}
