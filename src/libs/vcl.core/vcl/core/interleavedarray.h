/*
* This file is part of the Visual Computing Library (VCL) release under the
* MIT license.
*
* Copyright (c) 2014 Basil Fierz
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

// VCL config
#include <vcl/config/global.h>
#include <vcl/config/eigen.h>

// VCL
#include <vcl/core/contract.h>

namespace Vcl { namespace Core
{
	const int DynamicStride = -1;

	/*!
	 *	Storage class storing the given matrix objects in row-major order.
	 */
	template<typename SCALAR, int ROWS = 0, int COLS = 0, int STRIDE = 0>
	class InterleavedArray
	{
	public:	
		static const int Cols = COLS;
		static const int Rows = ROWS;
		static const int Stride = ((STRIDE == 0) || (STRIDE == 1)) ? 1 : STRIDE;

	public:
		/*!
		 *
		 *	\param size   Number of entries in the storage.
		 *	\param rows   Number of rows in a data element.
		 *	\param cols   Number of cols in a data element.
		 *	\param stride Defines the number elements being adjacent in memory.
		 *				  0: All data entries of an element are consecutive in memory.
		 *				  n: n elementy are grouped together. Note: 1 has the same effect as 0.
		 *				  Dynamic: The stride is the same as the number of elements in the container.
		 */
		InterleavedArray(size_t size, int rows = ROWS, int cols = COLS, int stride = STRIDE)
		: mSize(size), mRows(rows), mCols(cols), mStride(stride)
		{
			// Template configuration checks
			static_assert(COLS == DynamicStride || COLS > 0, "Width of a data member is either dynamic or fixed sized.");
			static_assert(ROWS == DynamicStride || ROWS > 0, "Height of a data member is either dynamic or fixed sized.");
			
			// Initialisation checks
			Require(rows > 0, "Number of rows is positive.");
			Require(cols > 0, "Number of cols is positive.");
			Require(stride == DynamicStride || stride >= 0, "Stride is Dynamic, 0 or greater 0");
			
			// Pad the requested size to the alignment
			// Note: This is done in order to support optimaly sized vector operations
			mAllocated = mSize;

			const size_t alignment = 32;
			if (mSize % alignment > 0)
				mAllocated += alignment - mSize % alignment;

			// Allocate initial memory
			mData = (SCALAR*) _mm_malloc(mAllocated*mRows*mCols*sizeof(SCALAR), alignment);
		}

		~InterleavedArray()
		{
			_mm_free(mData);
		}
		
	public:
		SCALAR* data() const
		{
			return mData;
		}

		size_t size() const
		{
			return mSize;
		}

		void setZero()
		{
			memset(mData, 0, mAllocated*mRows*mCols*sizeof(SCALAR));
		}

	public:
		template<typename SCALAR_OUT>
		Eigen::Map
		<
			Eigen::Matrix<SCALAR_OUT, ROWS, COLS>,
			Eigen::Unaligned,
			Eigen::Stride
			<
				((STRIDE == DynamicStride) ? DynamicStride : ((STRIDE == 0 || STRIDE == 1) ? ROWS : (ROWS*STRIDE / (sizeof(SCALAR_OUT) / sizeof(SCALAR))))),
				((STRIDE == DynamicStride) ? DynamicStride : ((STRIDE == 0 || STRIDE == 1) ? 1 : (STRIDE / (sizeof(SCALAR_OUT) / sizeof(SCALAR)))))
			>
		> at(int idx)
		{
			static_assert
			(
				(sizeof(SCALAR_OUT) >= sizeof(SCALAR)) && (sizeof(SCALAR_OUT) % sizeof(SCALAR) == 0),
				"Always access multiples of the internal type."
			);
			static_assert
			(
				implies(Stride != DynamicStride, (sizeof(SCALAR_OUT) / sizeof(SCALAR) <= Stride) && (Stride % (sizeof(SCALAR_OUT) / sizeof(SCALAR)) == 0)),
				"Output size and stride size are compatible."
			);

			typedef Eigen::Stride
			<
				((STRIDE == DynamicStride) ? DynamicStride : ((STRIDE == 0 || STRIDE == 1) ? ROWS : (ROWS*STRIDE / (sizeof(SCALAR_OUT) / sizeof(SCALAR))))),
				((STRIDE == DynamicStride) ? DynamicStride : ((STRIDE == 0 || STRIDE == 1) ? 1 : (STRIDE / (sizeof(SCALAR_OUT) / sizeof(SCALAR)))))
			> StrideType;
			
			Require
			(
				implies(Stride == DynamicStride, (sizeof(SCALAR_OUT) / sizeof(SCALAR) <= mAllocated) && (mAllocated % (sizeof(SCALAR_OUT) / sizeof(SCALAR)) == 0)),
				"Output size and stride size are compatible."
			);

			// Stride between to entries of the same matrix
			size_t stride = (mStride == DynamicStride) ? mAllocated : mStride;

			// Size of a single entry
			size_t scalar_width = sizeof(SCALAR_OUT) / sizeof(SCALAR);

			// Outer/Inner stride
			size_t outer_stride = 0;
			size_t inner_stride = 0;

			// Compute the address of the first entry
			auto base = reinterpret_cast<SCALAR_OUT*>(mData);
			if (stride == 0 || stride == 1)
			{
				base += idx*mRows*mCols;
			}
			else if (stride < mAllocated)
			{
				size_t entry = idx % stride;
				size_t group_idx = idx - entry;
				base += group_idx*mRows*mCols + entry;

				outer_stride = mRows*stride / scalar_width;
				inner_stride = stride / scalar_width;
			}
			else
			{
				base += idx;

				outer_stride = mRows*stride / scalar_width;
				inner_stride = stride / scalar_width;
			}

			if (Stride == DynamicStride || mStride == DynamicStride)
				return Eigen::Map<Eigen::Matrix<SCALAR_OUT, ROWS, COLS>, Eigen::Unaligned, StrideType>
				(
					base, StrideType(outer_stride, inner_stride)
				);
			else
				return Eigen::Map<Eigen::Matrix<SCALAR_OUT, ROWS, COLS>, Eigen::Unaligned, StrideType>(base);
		}

		template<typename SCALAR_OUT>
		const Eigen::Map
		<
			Eigen::Matrix<SCALAR_OUT, ROWS, COLS>,
			Eigen::Unaligned,
			Eigen::Stride
			<
				((STRIDE == DynamicStride) ? DynamicStride : ((STRIDE == 0 || STRIDE == 1) ? ROWS : (ROWS*STRIDE / (sizeof(SCALAR_OUT) / sizeof(SCALAR))))),
				((STRIDE == DynamicStride) ? DynamicStride : ((STRIDE == 0 || STRIDE == 1) ? 1 : (STRIDE / (sizeof(SCALAR_OUT) / sizeof(SCALAR)))))
			>
		> at(int idx) const
		{
			return const_cast<InterleavedArray*>(this)->at(idx);
		}
		
	private:
		SCALAR* mData;
		size_t mSize;
		size_t mAllocated;
		size_t mRows;
		size_t mCols;
		size_t mStride;
	};
}}