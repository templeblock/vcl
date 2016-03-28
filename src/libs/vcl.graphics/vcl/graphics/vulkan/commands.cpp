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
#include <vcl/graphics/vulkan/context.h>

namespace Vcl { namespace Graphics { namespace Vulkan
{
	Semaphore::Semaphore(Context* context)
	: _context(context)
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;

		VkResult res = vkCreateSemaphore(*_context, &semaphoreCreateInfo, nullptr, &_semaphore);
		Check(res == VK_SUCCESS, "Semaphore was created.");
	}

	Semaphore::~Semaphore()
	{
		vkDestroySemaphore(*_context, _semaphore, nullptr);
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

	void CommandQueue::submit
	(
		gsl::span<VkCommandBuffer> buffers,
		VkPipelineStageFlags* flags,
		gsl::span<VkSemaphore> waiting,
		gsl::span<VkSemaphore> signaling
	)
	{
		VkSubmitInfo info;
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.pNext = nullptr;
		info.commandBufferCount = uint32_t(buffers.size());
		info.pCommandBuffers = buffers.data();
		info.waitSemaphoreCount = uint32_t(waiting.size());
		info.pWaitSemaphores = waiting.data();
		info.pWaitDstStageMask = flags;
		info.signalSemaphoreCount = uint32_t(signaling.size());
		info.pSignalSemaphores = signaling.data();

		VkResult res = vkQueueSubmit(_queue, 1, &info, VK_NULL_HANDLE);
		Check(res == VK_SUCCESS, "Command buffer recording began.");
	}

	void CommandQueue::submit
	(
		const CommandBuffer& buffer
	)
	{
		VkCommandBuffer buf = buffer;
		submit({ buf });
	}

	void CommandQueue::waitIdle()
	{
		VkResult res = vkQueueWaitIdle(_queue);
		Check(res == VK_SUCCESS, "Command buffer recording began.");
	}
}}}
