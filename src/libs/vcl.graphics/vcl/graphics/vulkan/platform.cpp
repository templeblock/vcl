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
#include <vcl/graphics/vulkan/platform.h>

// C++ standard library
#include <array>
#include <iostream>

// VCL
#include <vcl/core/contract.h>

namespace Vcl { namespace Graphics { namespace Vulkan
{
	Platform* Platform::_implementation = nullptr;

	void Platform::initialise()
	{
		if (_implementation == nullptr)
			_implementation = new Platform();
	}

	Platform* Platform::instance()
	{
		Check(_implementation != nullptr, "Vulkan platorm is initialised.");
		return _implementation;
	}

	void Platform::dispose()
	{
		VCL_SAFE_DELETE(_implementation);
	}

	Platform::Platform()
	{
		VkResult res;

		// Instance information
		VkInstanceCreateInfo create_info;
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;

		// Information about the application
		create_info.pApplicationInfo = nullptr;

		// Enable additional layers
		create_info.enabledLayerCount = 0;
		create_info.ppEnabledLayerNames = nullptr;

		// Enable additional extensions
		create_info.enabledExtensionCount = 0;
		create_info.ppEnabledExtensionNames = nullptr;

		// Create the instance
		res = vkCreateInstance(&create_info, nullptr, &_instance);
		if (res != VkResult::VK_SUCCESS)
			throw "";

		// Enumerate the available devices
		uint32_t nr_devs = 0;
		res = vkEnumeratePhysicalDevices(_instance, &nr_devs, nullptr);
		if (res != VkResult::VK_SUCCESS || nr_devs == 0)
			throw "";

		std::vector<VkPhysicalDevice> devs(nr_devs);
		res = vkEnumeratePhysicalDevices(_instance, &nr_devs, devs.data());
		if (res != VkResult::VK_SUCCESS)
			throw "";

		// Add the devices to the platform
		for (auto dev : devs)
		{
			_devices.emplace_back(dev);
		}
	}

	Platform::~Platform()
	{
		_devices.clear();

		// Cleanup the platform
		vkDestroyInstance(_instance, nullptr);
	}

	int Platform::nrDevices() const
	{
		return static_cast<int>(_devices.size());
	}

	const Device& Platform::device(int idx) const
	{
		Require(idx < _devices.size(), "idx is valid.");

		return _devices[idx];
	}
}}}
