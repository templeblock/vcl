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

namespace Vcl { namespace Graphics { namespace Vulkan
{
	struct ContextQueueInfo
	{
		//! Index of the vulkan queue family
		uint32_t FamilyIndex;
	};

	class CommandQueue final
	{
	public:
		//! Constructor
		CommandQueue();

		//! Destructor
		~CommandQueue();

		//! Convert to Vulkan ID
		inline operator VkQueue() const
		{
			return _queue;
		}

	public:
		void submit();

		void waitIdle();

	private:
		//! Vulkan device queue
		VkQueue _queue;
	};

	class Context final
	{
	public:
		//! Constructor
		Context(VkPhysicalDevice dev, gsl::span<const char*> layers, gsl::span<const char*> extensions);

		//! Destructor
		~Context();

		//! Convert to Vulkan ID
		inline operator VkDevice() const
		{
			return _device;
		}

	public:
		VkQueue queue(uint32_t idx);
		
	private:
		//! Vulkan phyiscal device
		VkPhysicalDevice _physicalDevice{ nullptr };

		//! Vulkan device
		VkDevice _device{ nullptr };

		//! Queue families
	};
}}}
