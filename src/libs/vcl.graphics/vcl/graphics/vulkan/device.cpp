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
#include <vcl/graphics/vulkan/device.h>

// C++ standard library
#include <iostream>
#include <vector>

// VCL
#include <vcl/core/contract.h>

namespace Vcl { namespace Graphics { namespace Vulkan
{
	Device::Device(VkPhysicalDevice dev)
	: _device(dev)
	{
		// Properties of the device
		VkPhysicalDeviceProperties dev_props;
		vkGetPhysicalDeviceProperties(dev, &dev_props);

		// Enumerate the number of queue families
		uint32_t nr_queues = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(dev, &nr_queues, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(nr_queues);
		vkGetPhysicalDeviceQueueFamilyProperties(dev, &nr_queues, queue_families.data());

		// Enumerte the device features
		VkPhysicalDeviceFeatures dev_features;
		vkGetPhysicalDeviceFeatures(dev, &dev_features);

		// Store the relevant data
		_name = dev_props.deviceName;
	}
}}}
