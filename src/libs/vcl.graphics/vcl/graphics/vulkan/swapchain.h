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
#pragma once

// VCL configuration
#include <vcl/config/global.h>

// C++ standard library
#include <string>

// Vulkan
#include <vulkan/vulkan.h>

// GSL
#include <vcl/core/3rdparty/gsl/span.h>

// VCL
#include <vcl/graphics/vulkan/context.h>

namespace Vcl { namespace Graphics { namespace Vulkan
{
	class SwapChain final
	{
	public:
		/*!
		 *	\brief Constructor
		 *
		 *	\param surface Vulkan surface for which this swap-chain should be used
		 */
		SwapChain(VkInstance instance, VkPhysicalDevice device, VkDevice context, VkSurfaceKHR surface);

		//! Destructor
		~SwapChain();

		//! Convert to OpenCL device ID
		inline operator VkSwapchainKHR() const
		{
			return _swapchain;
		}

	private:
		//! Owner instance
		VkInstance _instance{ nullptr };
		
		//! Owner physical device
		VkPhysicalDevice _device{ nullptr };
		
		//! Owner device
		VkDevice _context{ nullptr };

		//! Surface of this swap chain
		VkSurfaceKHR _surface{ nullptr };
		
		//! Allocated swap-chain
		VkSwapchainKHR _swapchain{ nullptr };

		//! Selected colour format
		VkFormat _colourFormat;

		//! Selected colour space
		VkColorSpaceKHR _colourSpace;

	private:
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR{ nullptr };
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR{ nullptr };
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR{ nullptr };
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR{ nullptr };
		PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR{ nullptr };
		PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR{ nullptr };
		PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR{ nullptr };
		PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR{ nullptr };
		PFN_vkQueuePresentKHR vkQueuePresentKHR{ nullptr };
	};
}}}
