/*
 * This file is part of the Visual Computing Library (VCL) release under the
 * MIT license.
 *
 * Copyright (c) 2015 Basil Fierz
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
#include <memory>
#include <random>

// Include the relevant parts from the library
#include <vcl/core/interleavedarray.h>
#include <vcl/math/math.h>
#include <vcl/math/rotation33_torque.h>

// Google test
#include <gtest/gtest.h>

// Common functions
namespace
{
	template<typename Scalar>
	void createProblems
	(
		size_t nr_problems,
		Scalar max_angle,
		Vcl::Core::InterleavedArray<Scalar, 3, 3, -1>* F,
		Vcl::Core::InterleavedArray<Scalar, 3, 3, -1>* R
	)
	{
		// Random number generator
		std::mt19937_64 rng;
		std::uniform_real_distribution<float> d;
		std::uniform_real_distribution<float> a{ -max_angle, max_angle };
			
		// Initialize data
		for (int i = 0; i < (int) nr_problems; i++)
		{
			// Rest-state
			Eigen::Matrix<Scalar, 3, 3> X0;
			X0 << d(rng), d(rng), d(rng),
				  d(rng), d(rng), d(rng),
				  d(rng), d(rng), d(rng);

			// Rotation angle
			Scalar angle = a(rng);

			// Rotation axis
			Eigen::Matrix<Scalar, 3, 1> rot_vec;
			rot_vec << d(rng), d(rng), d(rng);
			rot_vec.normalize();

			// Rotation matrix
			Eigen::Matrix<Scalar, 3, 3> Rot = Eigen::AngleAxis<Scalar>{angle, rot_vec}.toRotationMatrix();
			R->template at<Scalar>(i) = Rot;

			Eigen::Matrix<Scalar, 3, 3> X = Rot * X0;
			F->template at<Scalar>(i) = X * X0.inverse();
		}
	}

	template<typename Scalar>
	void checkSolution
	(
		size_t nr_problems,
		Scalar tol,
		const Vcl::Core::InterleavedArray<Scalar, 3, 3, -1>& refRa,
		const Vcl::Core::InterleavedArray<Scalar, 3, 3, -1>& resRa
	)
	{
		using Vcl::Mathematics::equal;

		Eigen::IOFormat fmt(6, 0, ", ", ";", "[", "]");

		for (int i = 0; i < static_cast<int>(nr_problems); i++)
		{
			Vcl::Matrix3f refR = refRa.template at<Scalar>(i);
			Vcl::Matrix3f resR = resRa.template at<Scalar>(i);

			Scalar sqLenRefRc0 = refR.col(0).squaredNorm();
			Scalar sqLenRefRc1 = refR.col(1).squaredNorm();
			Scalar sqLenRefRc2 = refR.col(2).squaredNorm();
			EXPECT_TRUE(equal(sqLenRefRc0, Scalar(1), tol)) << "Reference R(" << i << "): Column 0 is not normalized.";
			EXPECT_TRUE(equal(sqLenRefRc1, Scalar(1), tol)) << "Reference R(" << i << "): Column 1 is not normalized.";
			EXPECT_TRUE(equal(sqLenRefRc2, Scalar(1), tol)) << "Reference R(" << i << "): Column 2 is not normalized.";

			Scalar sqLenResRc0 = resR.col(0).squaredNorm();
			Scalar sqLenResRc1 = resR.col(1).squaredNorm();
			Scalar sqLenResRc2 = resR.col(2).squaredNorm();
			EXPECT_TRUE(equal(sqLenResRc0, Scalar(1), tol)) << "Result R(" << i << "): Column 0 is not normalized.";
			EXPECT_TRUE(equal(sqLenResRc1, Scalar(1), tol)) << "Result R(" << i << "): Column 1 is not normalized.";
			EXPECT_TRUE(equal(sqLenResRc2, Scalar(1), tol)) << "Result R(" << i << "): Column 2 is not normalized.";

			//bool eqR = refR.array().abs().isApprox(resR.array().abs(), tol);
			//EXPECT_TRUE(eqR) << "R(" << i << ") - Ref: " << refR.format(fmt) << ", Actual: " << resR.format(fmt);

			float a_dist = Eigen::Quaternionf{ refR }.angularDistance(Eigen::Quaternionf{ resR });
			EXPECT_LE(a_dist, tol) << "Dist: " << a_dist;

			Vcl::Matrix3f I = resR.transpose() * resR;
			Vcl::Matrix3f Iref = Vcl::Matrix3f::Identity();

			EXPECT_TRUE(equal(Iref, I, tol)) << "Index: " << i << ", Result U^t U: not Identity.";
		}
	}
}
template<typename WideScalar>
void runRotationTest(float max_angle, float tol)
{
	using scalar_t = float;
	using real_t = WideScalar;
	using matrix3_t = Eigen::Matrix<real_t, 3, 3>;
	using vector3_t = Eigen::Matrix<real_t, 3, 1>;

	size_t nr_problems = 128;
	Vcl::Core::InterleavedArray<scalar_t, 3, 3, -1>    F(nr_problems);
	Vcl::Core::InterleavedArray<scalar_t, 3, 3, -1> resR(nr_problems);
	Vcl::Core::InterleavedArray<scalar_t, 3, 3, -1> refR(nr_problems);

	createProblems<scalar_t>(nr_problems, max_angle * 3.14f / 180.0f, &F, &refR);

	// Strides
	size_t stride = nr_problems;
	size_t width = sizeof(real_t) / sizeof(scalar_t);

	std::vector<int> iterations(128);
	for (int i = 0; i < static_cast<int>(stride / width); i++)
	{
		matrix3_t A = F.at<real_t>(i);
		matrix3_t R = A;

		iterations[i] = Vcl::Mathematics::Rotation(A, R);

		resR.at<real_t>(i) = R;
	}

	// Check against reference solution
	checkSolution(nr_problems, tol, refR, resR);
}

TEST(Rotation33, Rotation30Float)
{
	runRotationTest<float>(30.0f, 2e-5f);
}
TEST(Rotation33, Rotation60Float)
{
	runRotationTest<float>(60.0f, 2e-5f);
}
TEST(Rotation33, Rotation90Float)
{
	runRotationTest<float>(90.0f, 2e-5f);
}
TEST(Rotation33, Rotation120Float)
{
	runRotationTest<float>(120.0f, 2e-5f);
}
TEST(Rotation33, Rotation180Float)
{
	runRotationTest<float>(180.0f, 2e-5f);
}

TEST(Rotation33, Rotation30Float4)
{
	runRotationTest<Vcl::float4>(30.0f, 2e-5f);
}
TEST(Rotation33, Rotation60Float4)
{
	runRotationTest<Vcl::float4>(60.0f, 2e-5f);
}
TEST(Rotation33, Rotation90Float4)
{
	runRotationTest<Vcl::float4>(90.0f, 2e-5f);
}

TEST(Rotation33, Rotation30Float8)
{
	runRotationTest<Vcl::float8>(30.0f, 2e-5f);
}
TEST(Rotation33, Rotation60Float8)
{
	runRotationTest<Vcl::float8>(60.0f, 2e-5f);
}
TEST(Rotation33, Rotation90Float8)
{
	runRotationTest<Vcl::float8>(90.0f, 2e-5f);
}

TEST(Rotation33, Rotation30Float16)
{
	runRotationTest<Vcl::float16>(30.0f, 2e-5f);
}
TEST(Rotation33, Rotation60Float16)
{
	runRotationTest<Vcl::float16>(60.0f, 2e-5f);
}
TEST(Rotation33, Rotation90Float16)
{
	runRotationTest<Vcl::float16>(90.0f, 2e-5f);
}
