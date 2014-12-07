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

// VCL configuration
#include <vcl/config/global.h>

// VCL 
#include <vcl/core/simd/bool8_avx.h>
#include <vcl/core/simd/vectorscalar.h>
#include <vcl/core/simd/intrinsics_avx.h>

namespace Vcl
{
	template<>
	class VectorScalar<int, 8>
	{
	public:
		VCL_STRONG_INLINE VectorScalar() {}
		VCL_STRONG_INLINE VectorScalar(int s)
		{
			mF8 = _mm256_set1_epi32(s);
		}
		explicit VCL_STRONG_INLINE VectorScalar(int s0, int s1, int s2, int s3, int s4, int s5, int s6, int s7)
		{
			mF8 = _mm256_set_epi32(s0, s1, s2, s3, s4, s5, s6, s7);
		}
		explicit VCL_STRONG_INLINE VectorScalar(__m256i I4) : mF8(I4) {}

	public:
		VCL_STRONG_INLINE VectorScalar<int, 8>& operator= (const VectorScalar<int, 8>& rhs) { mF8 = rhs.mF8; return *this; }

	public:
		int& operator[] (int idx)
		{
			Require(0 <= idx && idx < 8, "Access is in range.");

			return mF8.m256i_i32[idx];
		}

		int operator[] (int idx) const
		{
			Require(0 <= idx && idx < 8, "Access is in range.");

			return mF8.m256i_i32[idx];
		}

		explicit operator __m256i() const
		{
			return mF8;
		}

	public:
		VCL_STRONG_INLINE VectorScalar<int, 8> operator+ (const VectorScalar<int, 8>& rhs) const { return VectorScalar<int, 8>(_mm256_add_epi32(mF8, rhs.mF8)); }
		VCL_STRONG_INLINE VectorScalar<int, 8> operator- (const VectorScalar<int, 8>& rhs) const { return VectorScalar<int, 8>(_mm256_sub_epi32(mF8, rhs.mF8)); }
		VCL_STRONG_INLINE VectorScalar<int, 8> operator* (const VectorScalar<int, 8>& rhs) const { return VectorScalar<int, 8>(_mm256_mullo_epi32(mF8, rhs.mF8)); }

	public:
		VCL_STRONG_INLINE VectorScalar<int, 8> abs() const { return VectorScalar<int, 8>(_mm256_abs_epi32(mF8)); }
		VCL_STRONG_INLINE VectorScalar<int, 8> max(const VectorScalar<int, 8>& rhs) const { return VectorScalar<int, 8>(_mm256_max_epi32(mF8, rhs.mF8)); }

	public:
		VCL_STRONG_INLINE VectorScalar<bool, 8> operator== (const VectorScalar<int, 8>& rhs) const
		{
			return VectorScalar<bool, 8>(_mm256_cmpeq_epi32(mF8, rhs.mF8));
		}

		VCL_STRONG_INLINE VectorScalar<bool, 8> operator< (const VectorScalar<int, 8>& rhs) const
		{
			return VectorScalar<bool, 8>(_mm256_cmplt_epi32(mF8, rhs.mF8));
		}
		VCL_STRONG_INLINE VectorScalar<bool, 8> operator<= (const VectorScalar<int, 8>& rhs) const
		{
			return VectorScalar<bool, 8>(_mm256_cmple_epi32(mF8, rhs.mF8));
		}
		VCL_STRONG_INLINE VectorScalar<bool, 8> operator> (const VectorScalar<int, 8>& rhs) const
		{
			return VectorScalar<bool, 8>(_mm256_cmpgt_epi32(mF8, rhs.mF8));
		}
		VCL_STRONG_INLINE VectorScalar<bool, 8> operator>= (const VectorScalar<int, 8>& rhs) const
		{
			return VectorScalar<bool, 8>(_mm256_cmpge_epi32(mF8, rhs.mF8));
		}

	public:
		friend std::ostream& operator<< (std::ostream &s, const VectorScalar<int, 8>& rhs);
		friend VectorScalar<int, 8> select(const VectorScalar<bool, 8>& mask, const VectorScalar<int, 8>& a, const VectorScalar<int, 8>& b);
		friend VectorScalar<int, 8> signum(const VectorScalar<int, 8>& a);

	private:
		__m256i mF8;
	};

	VCL_STRONG_INLINE VectorScalar<int, 8> select(const VectorScalar<bool, 8>& mask, const VectorScalar<int, 8>& a, const VectorScalar<int, 8>& b)
	{
		// (((b ^ a) & mask)^b)
		return VectorScalar<int, 8>(_mm256_xor_si256(b.mF8, _mm256_and_si256(_mm256_castps_si256(mask.mF8), _mm256_xor_si256(b.mF8, a.mF8))));
	}

	VCL_STRONG_INLINE VectorScalar<int, 8> signum(const VectorScalar<int, 8>& a)
	{
		return VectorScalar<int, 8>
		(
			_mm256_and_si256
			(
				_mm256_or_si256
				(
					_mm256_and_si256(a.mF8, _mm256_set1_epi32(0x80000000)), _mm256_set1_epi32(1)
				), _mm256_cmpneq_epi32(a.mF8, _mm256_setzero_si256())
			)
		);
	}

	VCL_STRONG_INLINE std::ostream& operator<< (std::ostream &s, const VectorScalar<int, 8>& rhs)
	{
		int VCL_ALIGN(32) vars[8];
		_mm256_store_si256((__m256i*) (vars + 0), rhs.mF8);

		s << "'" << vars[0] << "," << vars[1] << "," << vars[2] << "," << vars[3]
			     << vars[4] << "," << vars[5] << "," << vars[6] << "," << vars[7] << "'";

		return s;
	}
}
