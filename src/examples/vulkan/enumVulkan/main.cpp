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

// VCL
#include <vcl/graphics/vulkan/platform.h>

int main(int argc, char* argv[])
{
	Vcl::Graphics::Vulkan::Platform::initialise();
	auto platform = Vcl::Graphics::Vulkan::Platform::instance();
	auto dev_config = platform->device(0);

	VkDeviceCreateInfo dev_info;
	dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dev_info.pNext = nullptr;
	dev_info.flags = 0;

	// Queue info
	float queuePriorities[] = { 0.0f, 0.0f };

	VkDeviceQueueCreateInfo queue_info[1];
	queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info[0].pNext = nullptr;
	queue_info[0].flags = 0;
	queue_info[0].queueCount = 1;
	queue_info[0].queueFamilyIndex = 0;
	queue_info[0].pQueuePriorities = queuePriorities;

	dev_info.queueCreateInfoCount = 1;
	dev_info.pQueueCreateInfos = queue_info;

	// Enable additional layers
	dev_info.enabledLayerCount = 0;
	dev_info.ppEnabledLayerNames = nullptr;

	// Enable additional extensions
	dev_info.enabledExtensionCount = 0;
	dev_info.ppEnabledExtensionNames = nullptr;

	// Enable features
	dev_info.pEnabledFeatures = nullptr;


	VkDevice dev;
	VkResult res = vkCreateDevice(dev_config, &dev_info, nullptr, &dev);
	if (res != VkResult::VK_SUCCESS)
		return 1;

	return 0;
}
