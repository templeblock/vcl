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
#include <vcl/graphics/vulkan/device.h>
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

	SwapChain::SwapChain(Context* context, VkCommandBuffer cmd_buffer, const SwapChainDescription& desc)
	: _context(context)
	, _desc(desc)
	{
		VkResult res;

		VCL_VK_GET_DEVICE_PROC(*context, vkCreateSwapchainKHR);
		VCL_VK_GET_DEVICE_PROC(*context, vkDestroySwapchainKHR);
		VCL_VK_GET_DEVICE_PROC(*context, vkGetSwapchainImagesKHR);
		VCL_VK_GET_DEVICE_PROC(*context, vkAcquireNextImageKHR);
		VCL_VK_GET_DEVICE_PROC(*context, vkQueuePresentKHR);

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
		sc_create_info.pQueueFamilyIndices = nullptr;
		sc_create_info.presentMode = desc.PresentMode;
		sc_create_info.oldSwapchain = nullptr;//oldSwapchain;
		sc_create_info.clipped = true;
		sc_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		res = vkCreateSwapchainKHR(*context, &sc_create_info, nullptr, &_swapchain);
		Check(res == VK_SUCCESS, "Swap chain was created.");

		uint32_t nr_images = 0;
		res = vkGetSwapchainImagesKHR(*context, _swapchain, &nr_images, nullptr);
		Check(res == VK_SUCCESS, "Number of images can be queried.");

		// Ask the implementation for the swap-chain images
		_images.resize(nr_images);
		res = vkGetSwapchainImagesKHR(*context, _swapchain, &nr_images, _images.data());
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

			res = vkCreateImageView(*_context, &colorAttachmentView, nullptr, &_views[i]);
			Check(res == VK_SUCCESS, "Image view was created.", "Image index: {}", i);
		}
	}

	SwapChain::~SwapChain()
	{
		for (uint32_t i = 0; i < _images.size(); i++)
			vkDestroyImageView(*_context, _views[i], nullptr);

		if (_swapchain)
			vkDestroySwapchainKHR(*_context, _swapchain, nullptr);
	}

	VkResult SwapChain::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* currentBuffer)
	{
		return vkAcquireNextImageKHR(*_context, _swapchain, UINT64_MAX, presentCompleteSemaphore, (VkFence)nullptr, currentBuffer);
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

	Backbuffer::Backbuffer(SwapChain* swapchain, VkCommandBuffer cmd_buffer, uint32_t width, uint32_t height, VkFormat depth_format)
	: _swapchain(swapchain)
	{
		createDefaultRenderPass(VK_FORMAT_B8G8R8A8_UNORM, depth_format);
		createDepthBuffer(cmd_buffer, width, height, depth_format);
		createFramebuffers(width, height);
	}

	Backbuffer::~Backbuffer()
	{
		vkDestroyRenderPass(*_swapchain->context(), _renderPass, nullptr);

		vkDestroyImageView(*_swapchain->context(), _depthBufferView, nullptr);
		vkDestroyImage(*_swapchain->context(), _depthBufferImage, nullptr);
		vkFreeMemory(*_swapchain->context(), _depthBufferMemory, nullptr);
	}

	void Backbuffer::createDefaultRenderPass(VkFormat color_format, VkFormat depth_format)
	{
		VkAttachmentDescription attachments[2];
		attachments[0].format = color_format;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		attachments[1].format = depth_format;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.flags = 0;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorReference;
		subpass.pResolveAttachments = nullptr;
		subpass.pDepthStencilAttachment = &depthReference;
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = nullptr;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.pNext = nullptr;
		info.attachmentCount = 2;
		info.pAttachments = attachments;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 0;
		info.pDependencies = nullptr;

		VkResult res = vkCreateRenderPass(*_swapchain->context(), &info, nullptr, &_renderPass);
		Check(res == VK_SUCCESS, "Render-pass was created.");
	}

	void Backbuffer::createFramebuffers(uint32_t width, uint32_t height)
	{
		VkImageView attachments[2];

		// Depth/Stencil attachment is the same for all frame buffers
		attachments[1] = _depthBufferView;

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = nullptr;
		frameBufferCreateInfo.renderPass = _renderPass;
		frameBufferCreateInfo.attachmentCount = 2;
		frameBufferCreateInfo.pAttachments = attachments;
		frameBufferCreateInfo.width = width;
		frameBufferCreateInfo.height = height;
		frameBufferCreateInfo.layers = 1;

		// Create frame buffers for every swap chain image
		_framebuffers.resize(_swapchain->nrImages());
		for (uint32_t i = 0; i < _framebuffers.size(); i++)
		{
			attachments[0] = _swapchain->view(i);
			VkResult res = vkCreateFramebuffer(*_swapchain->context(), &frameBufferCreateInfo, nullptr, &_framebuffers[i]);
			Check(res == VK_SUCCESS, "Framebuffer was created.", "Framebuffer: {}", i);
		}
	}

	void Backbuffer::createDepthBuffer(VkCommandBuffer cmd_buffer, uint32_t width, uint32_t height, VkFormat depth_format)
	{
		VkResult res;

		VkImageCreateInfo image = {};
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.pNext = nullptr;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = depth_format;
		image.extent = { width, height, 1 };
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		image.flags = 0;

		res = vkCreateImage(*_swapchain->context(), &image, nullptr, &_depthBufferImage);
		Check(res == VK_SUCCESS, "Image was created.");

		VkMemoryAllocateInfo mem_alloc = {};
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = nullptr;
		mem_alloc.memoryTypeIndex = 0;

		VkMemoryRequirements mem_reqs;
		vkGetImageMemoryRequirements(*_swapchain->context(), _depthBufferImage, &mem_reqs);
		mem_alloc.allocationSize = mem_reqs.size;

		Device* dev = _swapchain->context()->device();
		mem_alloc.memoryTypeIndex = dev->getMemoryTypeIndex(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		res = vkAllocateMemory(*_swapchain->context(), &mem_alloc, nullptr, &_depthBufferMemory);
		Check(res == VK_SUCCESS, "Image memory allocated.");

		res = vkBindImageMemory(*_swapchain->context(), _depthBufferImage, _depthBufferMemory, 0);
		Check(res == VK_SUCCESS, "Image was bound to memory.");

		setImageLayout
		(
			cmd_buffer,
			_depthBufferImage,
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);

		VkImageViewCreateInfo view = {};
		view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view.pNext = nullptr;
		view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view.format = depth_format;
		view.flags = 0;
		view.image = _depthBufferImage;
		view.subresourceRange = {};
		view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		view.subresourceRange.baseMipLevel = 0;
		view.subresourceRange.levelCount = 1;
		view.subresourceRange.baseArrayLayer = 0;
		view.subresourceRange.layerCount = 1;
		
		res = vkCreateImageView(*_swapchain->context(), &view, nullptr, &_depthBufferView);
		Check(res == VK_SUCCESS, "Image memory allocated.");
	}
}}}