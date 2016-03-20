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
#include <vcl/graphics/vulkan/Context.h>

// C++ standard library
#include <iostream>
#include <vector>

// VCL
#include <vcl/core/contract.h>

namespace Vcl { namespace Graphics { namespace Vulkan
{
	Context::Context(VkPhysicalDevice dev, gsl::span<const char*> layers, gsl::span<const char*> extensions)
	: _physicalDevice(dev)
	{
		// Properties of the device
		VkDeviceCreateInfo dev_info;
		dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		dev_info.pNext = nullptr;
		dev_info.flags = 0;

		// Enable additional layers
		dev_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
		dev_info.ppEnabledLayerNames = layers.data();

		// Enable additional extensions
		dev_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		dev_info.ppEnabledExtensionNames = extensions.data();

		// Enable features
		dev_info.pEnabledFeatures = nullptr;


		// Queue info
		float queuePriorities[] = { 0.0f, 0.0f };

		VkDeviceQueueCreateInfo queue_info[1];
		queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info[0].pNext = nullptr;
		queue_info[0].flags = 0;
		queue_info[0].queueCount = 2;
		queue_info[0].queueFamilyIndex = 0;
		queue_info[0].pQueuePriorities = queuePriorities;

		dev_info.queueCreateInfoCount = 1;
		dev_info.pQueueCreateInfos = queue_info;


		VkResult res = vkCreateDevice(_physicalDevice, &dev_info, nullptr, &_device);
		if (res != VkResult::VK_SUCCESS)
			throw "";
	}

	Context::~Context()
	{
		vkDestroyDevice(_device, nullptr);
	}
}}}
