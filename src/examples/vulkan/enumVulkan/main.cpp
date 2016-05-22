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
#include <vcl/config/eigen.h>

// C++ standard library
#include <iostream>
#include <vector>

// Vulkan API
#include <vulkan/vulkan.h>

// GLFW
#include <GLFW/glfw3.h>

// VCL
#include <vcl/graphics/runtime/vulkan/resource/shader.h>
#include <vcl/graphics/vulkan/commands.h>
#include <vcl/graphics/vulkan/pipelinestate.h>
#include <vcl/graphics/vulkan/platform.h>
#include <vcl/graphics/vulkan/swapchain.h>
#include <vcl/graphics/vulkan/tools.h>

// Load a binary file into a buffer (e.g. SPIR-V)
char *readBinaryFile(const char *filename, size_t *psize)
{
	long int size;
	size_t retval;
	void *shader_code;

	FILE *fp = fopen(filename, "rb");
	if (!fp) return NULL;

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);

	shader_code = malloc(size);
	retval = fread(shader_code, size, 1, fp);
	assert(retval == 1);

	*psize = size;

	return (char*)shader_code;
}

void buildCommandBuffers(Vcl::Graphics::Vulkan::Backbuffer* bb, VkCommandPool cmdPool, VkRenderPass renderPass, uint32_t width, uint32_t height, std::vector<VkCommandBuffer>& drawCmdBuffers)
{
	drawCmdBuffers.resize(bb->swapChain()->nrImages());

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = cmdPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = (uint32_t)drawCmdBuffers.size();

	VkResult vkRes = vkAllocateCommandBuffers(*bb->context(), &commandBufferAllocateInfo, drawCmdBuffers.data());
	assert(!vkRes);

	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext = NULL;

	VkClearValue clearValues[2];
	clearValues[0].color = { 1.0f, 0.0f, 1.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = NULL;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	VkResult err;

	for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
	{
		// Set target frame buffer
		renderPassBeginInfo.framebuffer = bb->framebuffer(i);

		err = vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo);
		assert(!err);

		vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Update dynamic viewport state
		VkViewport viewport = {};
		viewport.x = (float)width / 4;
		viewport.y = (float)height / 4;
		viewport.height = (float)height / 2;
		viewport.width = (float)width / 2;
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

		// Update dynamic scissor state
		VkRect2D scissor = {};
		scissor.extent.width = width / 2;
		scissor.extent.height = height / 2;
		scissor.offset.x = width / 4;
		scissor.offset.y = height / 4;
		vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

		//// Bind descriptor sets describing shader binding points
		//vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
		//
		//// Bind the rendering pipeline (including the shaders)
		//vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.solid);
		//
		//// Bind triangle vertices
		//VkDeviceSize offsets[1] = { 0 };
		//vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &vertices.buf, offsets);
		//
		//// Bind triangle indices
		//vkCmdBindIndexBuffer(drawCmdBuffers[i], indices.buf, 0, VK_INDEX_TYPE_UINT32);
		//
		//// Draw indexed triangle
		//vkCmdDrawIndexed(drawCmdBuffers[i], indices.count, 1, 0, 0, 1);

		vkCmdEndRenderPass(drawCmdBuffers[i]);

		// Add a present memory barrier to the end of the command buffer
		// This will transform the frame buffer color attachment to a
		// new layout for presenting it to the windowing system integration 
		VkImageMemoryBarrier prePresentBarrier = {};
		prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		prePresentBarrier.pNext = NULL;
		prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		prePresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		prePresentBarrier.image = bb->swapChain()->image(i);

		VkImageMemoryBarrier *pMemoryBarrier = &prePresentBarrier;
		vkCmdPipelineBarrier
		(
			drawCmdBuffers[i],
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &prePresentBarrier
		);

		err = vkEndCommandBuffer(drawCmdBuffers[i]);
		assert(!err);
	}
}


class GlfwInstance
{
public:
	GlfwInstance()
	{
		glfwSetErrorCallback(errorCallback);
		_isInitialized = glfwInit() != 0 ? true : false;
		if (!_isInitialized)
			throw std::runtime_error{ "Unexpected error when initializing GLFW" };

		// Since we are using vulkan we do not need any predefined client API
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Check Vulkan support
		_vulkanSupport = glfwVulkanSupported() != 0 ? true : false;
		
		unsigned int nr_exts = 0;
		auto exts = glfwGetRequiredInstanceExtensions(&nr_exts);
		_vulkanExtensions = { exts, nr_exts };
	}

	~GlfwInstance()
	{
		if (_isInitialized)
			glfwTerminate();
	}

public:
	//! \returns true if vulkan is supported
	bool isVulkanSupported() const { return _vulkanSupport; }

	//! \returns the required vulkan extensions
	gsl::span<const char*> vulkanExtensions() const { return _vulkanExtensions; }

private:
	static void errorCallback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}

private:
	bool _isInitialized{ false };

	//! Query if Vulkan is supported
	bool _vulkanSupport{ false };

	//! Required Vulkan extensions
	gsl::span<const char*> _vulkanExtensions;
};


int main(int argc, char* argv[])
{
	using namespace Vcl::Graphics::Vulkan;

	GlfwInstance glfw;
	if (!glfw.isVulkanSupported())
	{
		std::cerr << "Vulkan is not supported" << std::endl;
		return 1;
	}

	// Vulkan extension
	std::array<const char*, 1> context_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	// Initialize the Vulkan platform
	auto platform = std::make_unique<Platform>(glfw.vulkanExtensions());
	auto device = platform->device(0);
	auto context = device.createContext(context_extensions);
	CommandQueue queue{ context->queue(0) };
	CommandBuffer setup_buffer{ *context, context->commandPool(0, CommandBufferType::Default) };
	CommandBuffer post_present{ *context, context->commandPool(0, CommandBufferType::Default) };

	// Create a window
	auto window = glfwCreateWindow(1280, 720, "Vulkan Demo", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Cannot create a window in which to draw!" << std::endl;
		return 1;
	}
	
	// Begin: Setup surface and swap-chain

	// Create a WSI surface for the window
	VkSurfaceKHR surface_ctx;
	glfwCreateWindowSurface(*platform, window, nullptr, &surface_ctx);

	// Create a swap-chain
	Surface surface{ *platform, device, surface_ctx };

	SwapChainDescription desc;
	desc.Surface = surface;
	desc.NumberOfImages = 1;
	desc.ColourFormat = VK_FORMAT_B8G8R8A8_UNORM;
	desc.ColourSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	desc.Width = 1280;
	desc.Height = 720;
	desc.PresentMode = VK_PRESENT_MODE_FIFO_KHR;
	desc.PreTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

	VkRenderPass render_pass = createDefaultRenderPass(*context, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT);

	setup_buffer.begin();
	SwapChain swapchain{ context.get(), setup_buffer, desc };
	Backbuffer framebuffer{ &swapchain, render_pass, setup_buffer, 1280, 720, VK_FORMAT_D32_SFLOAT_S8_UINT };
	setup_buffer.end();
	queue.submit(setup_buffer);
	queue.waitIdle();
	
	// End: Setup surface and swap-chain

	// Begin: Scene setup

	// Descriptor set
	DescriptorSetLayout descriptor_set_layout = 
	{
		context.get(),
		{
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0 }
		}
	};

	// Create a descriptor-set and pipeline layout
	PipelineLayout layout{ context.get(), &descriptor_set_layout };

	// Create the pipeline state	
	Vcl::Graphics::Runtime::InputLayoutDescription opaqueTetraLayout =
	{
		{
			{ 0, sizeof(Eigen::Vector4i), Vcl::Graphics::Runtime::VertexDataClassification::VertexDataPerObject },
			{ 1, sizeof(Eigen::Vector4f), Vcl::Graphics::Runtime::VertexDataClassification::VertexDataPerObject }
		},
		{
			{ "Position",  Vcl::Graphics::SurfaceFormat::R32G32B32_FLOAT, 0, 0, 0 },
			{ "Colour", Vcl::Graphics::SurfaceFormat::R32G32B32_FLOAT, 0, 1, 0 }
		}
	};
	Vcl::Graphics::Runtime::PipelineStateDescription pipeline_desc;
	pipeline_desc.InputAssembly.PrimitiveRestartEnable = false;
	pipeline_desc.InputAssembly.Topology = Vcl::Graphics::Runtime::PrimitiveType::Pointlist;
	pipeline_desc.InputLayout = opaqueTetraLayout;

	size_t vs_code_size = 0;
	const char* vs_code = readBinaryFile("triangle.vert.spv", &vs_code_size);
	Vcl::Graphics::Runtime::Vulkan::Shader vs{ *context, Vcl::Graphics::Runtime::ShaderType::VertexShader, 0, vs_code, vs_code_size };
	pipeline_desc.VertexShader = &vs;

	PipelineState state{ context.get(), &layout, render_pass, pipeline_desc };

	// Build command buffers
	std::vector<VkCommandBuffer> cmds;
	buildCommandBuffers(&framebuffer, context->commandPool(0, CommandBufferType::Default), render_pass, 1280, 720, cmds);

	// End: Scene setup

	// Enter the event-loop
	Semaphore presentComplete{ context.get() };
	Semaphore renderComplete{ context.get() };
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		VkResult err;

		// Render the scene
		uint32_t curr_buf;
		swapchain.acquireNextImage(presentComplete, &curr_buf);
		// Add a post present image memory barrier
		// This will transform the frame buffer color attachment back
		// to it's initial layout after it has been presented to the
		// windowing system
		VkImageMemoryBarrier postPresentBarrier = {};
		postPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		postPresentBarrier.pNext = NULL;
		postPresentBarrier.srcAccessMask = 0;
		postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		postPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		postPresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		postPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		postPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		postPresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		postPresentBarrier.image = framebuffer.swapChain()->image(curr_buf);

		// Put post present barrier into command buffer
		post_present.begin();
		vkCmdPipelineBarrier
		(
			post_present,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &postPresentBarrier
		);
		post_present.end();

		// Submit to the queue
		queue.submit(post_present);
		queue.waitIdle();

		// Submit to the graphics queue
		VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		VkSemaphore s0 = presentComplete;
		VkSemaphore s1 = renderComplete;
		queue.submit({ cmds[curr_buf] }, &pipelineStages, { s0 }, { s1 });

		// Present the current buffer to the swap chain
		// We pass the signal semaphore from the submit info
		// to ensure that the image is not rendered until
		// all commands have been submitted
		swapchain.queuePresent(queue, curr_buf, renderComplete);
	}

	return 0;
}
