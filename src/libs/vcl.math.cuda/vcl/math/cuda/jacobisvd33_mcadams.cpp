/*
 * This file is part of the Visual Computing Library (VCL) release under the
 * MIT license.
 *
 * Copyright (c) 2014-2015 Basil Fierz
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
#include <vcl/math/cuda/jacobisvd33_mcadams.h>

// C++ standard library
#include <array>

// VCL
#include <vcl/compute/cuda/buffer.h>
#include <vcl/compute/cuda/kernel.h>
#include <vcl/compute/cuda/module.h>
#include <vcl/math/ceil.h>

// Kernels
extern uint32_t JacobiSVD33McAdamsCU[];
extern size_t JacobiSVD33McAdamsCUSize;

namespace Vcl { namespace Mathematics { namespace Cuda
{
	JacobiSVD33::JacobiSVD33(Core::ref_ptr<Compute::Context> ctx)
	: _ownerCtx(ctx)
	{
		Require(ctx, "Context is valid.");

		_A = ctx->createBuffer(Vcl::Compute::BufferAccess::ReadWrite, 16 * 9 * sizeof(float));
		_U = ctx->createBuffer(Vcl::Compute::BufferAccess::ReadWrite, 16 * 9 * sizeof(float));
		_V = ctx->createBuffer(Vcl::Compute::BufferAccess::ReadWrite, 16 * 9 * sizeof(float));
		_S = ctx->createBuffer(Vcl::Compute::BufferAccess::ReadWrite, 16 * 3 * sizeof(float));

		// Load the module
		_svdModule = ctx->createModuleFromSource(reinterpret_cast<const int8_t*>(JacobiSVD33McAdamsCU), JacobiSVD33McAdamsCUSize * sizeof(uint32_t));

		if (_svdModule)
		{
			// Load the jacobi SVD kernel
			_svdKernel = _svdModule->kernel("JacobiSVD33McAdams");
		}

		Ensure(implies(_svdModule, _svdKernel), "SVD kernel is valid.");
	}

	void JacobiSVD33::operator()
	(
		Vcl::Compute::CommandQueue& queue,
		const Vcl::Core::InterleavedArray<float, 3, 3, -1>& inA,
		Vcl::Core::InterleavedArray<float, 3, 3, -1>& outU,
		Vcl::Core::InterleavedArray<float, 3, 3, -1>& outV,
		Vcl::Core::InterleavedArray<float, 3, 1, -1>& outS
	)
	{
		Require(dynamic_cast<Compute::Cuda::CommandQueue*>(&queue), "Commandqueue is CUDA command queue.");
		Require(_svdKernel, "SVD kernel is loaded.");
		Require(inA.size() % 16 == 0, "Size of input is multiple of 16.");

		size_t numEntries = inA.size();

		auto A = Vcl::Core::dynamic_pointer_cast<Compute::Cuda::Buffer>(_A);
		auto U = Vcl::Core::dynamic_pointer_cast<Compute::Cuda::Buffer>(_U);
		auto V = Vcl::Core::dynamic_pointer_cast<Compute::Cuda::Buffer>(_V);
		auto S = Vcl::Core::dynamic_pointer_cast<Compute::Cuda::Buffer>(_S);

		if (_size < numEntries)
		{
			_size = ceil<16>(numEntries);

			A->resize(_size * 9 * sizeof(float));
			U->resize(_size * 9 * sizeof(float));
			V->resize(_size * 9 * sizeof(float));
			S->resize(_size * 3 * sizeof(float));
		}

		queue.write(A, inA.data());

		// Perform the SVD computation
		dim3 grid{ static_cast<unsigned int>(ceil<128>(numEntries)) };
		dim3 block{ 128};
		static_cast<Compute::Cuda::Kernel*>(_svdKernel.get())->run
		(
			static_cast<Compute::Cuda::CommandQueue&>(queue),
			grid,
			block,
			0,
			(int) numEntries,
			(int) _size,
			A,
			U,
			V,
			S
		);

		queue.read(outU.data(), U);
		queue.read(outV.data(), V);
		queue.read(outS.data(), S);
	}
}}}
