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
#include <vcl/graphics/vulkan/commands.h>

// C++ standard library
#include <iostream>
#include <vector>

// VCL
#include <vcl/core/contract.h>

namespace Vcl { namespace Graphics { namespace Vulkan
{
	CommandPool::CommandPool(VkDevice device, uint32_t queue_family)
	: _device(device)
	{
		VkCommandPoolCreateInfo cmd_pool_info;
		cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmd_pool_info.pNext = nullptr;
		cmd_pool_info.queueFamilyIndex = queue_family;
		cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VkResult res = vkCreateCommandPool(device, &cmd_pool_info, nullptr, &_pool);
		Check(res == VK_SUCCESS, "Command pool was created.");
	}
	CommandPool::~CommandPool()
	{
		vkDestroyCommandPool(_device, _pool, nullptr);
	}

	CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool pool)
	: _device(device)
	, _pool(pool)
	{
		VkCommandBufferAllocateInfo info;
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.commandPool = pool;
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		info.commandBufferCount = 1;

		VkResult res = vkAllocateCommandBuffers(device, &info, &_cmdBuffer);
		Check(res == VK_SUCCESS, "Command buffer was created.");
	}

	CommandBuffer::~CommandBuffer()
	{
		vkFreeCommandBuffers(_device, _pool, 1, &_cmdBuffer);
	}

	void CommandBuffer::begin()
	{
		VkCommandBufferBeginInfo info;
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.pInheritanceInfo = nullptr;

		VkResult res = vkBeginCommandBuffer(_cmdBuffer, &info);
		Check(res == VK_SUCCESS, "Command buffer recording began.");
	}

	void CommandBuffer::end()
	{
		VkResult res = vkEndCommandBuffer(_cmdBuffer);
		Check(res == VK_SUCCESS, "Command buffer recording ended.");
	}

	CommandQueue::CommandQueue(VkQueue queue)
	: _queue(queue)
	{

	}

	CommandQueue::~CommandQueue()
	{

	}

	void CommandQueue::submit(const CommandBuffer& buffer)
	{
		VkCommandBuffer cb = buffer;

		VkSubmitInfo info;
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.pNext = nullptr;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &cb;
		info.waitSemaphoreCount = 0;
		info.pWaitSemaphores = nullptr;
		info.pWaitDstStageMask = nullptr;
		info.signalSemaphoreCount = 0;
		info.pSignalSemaphores = nullptr;

		VkResult res = vkQueueSubmit(_queue, 1, &info, VK_NULL_HANDLE);
		Check(res == VK_SUCCESS, "Command buffer recording began.");
	}

	void CommandQueue::waitIdle()
	{
		VkResult res = vkQueueWaitIdle(_queue);
		Check(res == VK_SUCCESS, "Command buffer recording began.");
	}
}}}
