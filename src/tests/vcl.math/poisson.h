/*
 * This file is part of the Visual Computing Library (VCL) release under the
 * MIT license.
 *
 * Copyright (c) 2014-2016 Basil Fierz
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
#include <vcl/config/eigen.h>

// VCL
#include <vcl/math/math.h>

// Google test
#include <gtest/gtest.h>

// Example with exact solution taken from
// Burkardt - Jacobi Iterative Solution of Poisson's Equation in 1D
inline unsigned int createPoisson1DProblem(float& h, Eigen::VectorXf& rhs, Eigen::VectorXf& sol)
{
	// Number of points
	unsigned int nr_pts = 16;

	// Domain [0, 1]
	h = 1.0f / static_cast<float>(nr_pts - 1);

	// Right-hand side and exact solution of the poisson problem
	rhs.setZero(nr_pts);
	sol.setZero(nr_pts);
	for (Eigen::VectorXf::Index i = 0; i < static_cast<Eigen::VectorXf::Index>(nr_pts); i++)
	{
		float x = i * h;
		rhs(i) = -x * (x + 3) * exp(x);
		sol(i) =  x * (x - 1) * exp(x);
	}

	return nr_pts;
}

inline unsigned int createPoisson2DProblem(float& h, Eigen::VectorXf& rhs, Eigen::VectorXf& sol)
{
	using Vcl::Mathematics::pi;

	// Number of points
	unsigned int nr_pts = 16;

	// Domain [0, 1] x [0, 1]
	h = 1.0f / static_cast<float>(nr_pts - 1);

	// Right-hand side and exact solution of the poisson problem
	rhs.setZero(nr_pts*nr_pts);
	sol.setZero(nr_pts*nr_pts);
	for (Eigen::VectorXf::Index j = 0; j < static_cast<Eigen::VectorXf::Index>(nr_pts); j++)
	{
		for (Eigen::VectorXf::Index i = 0; i < static_cast<Eigen::VectorXf::Index>(nr_pts); i++)
		{
			float x = 0 + i * h;
			float y = 0 + j * h;
			rhs(j*nr_pts + i) = 8*3.14159f*3.14159f * sin(2*3.14159f * x) * sin(2*3.14159f * y);
			sol(j*nr_pts + i) = sin(2 * 3.14159f * x) * sin(2 * 3.14159f * y);
		}
	}

	return nr_pts;
}

inline unsigned int createPoisson3DProblem(float& h, Eigen::VectorXf& rhs, Eigen::VectorXf& sol)
{
	using Vcl::Mathematics::pi;

	// Number of points
	unsigned int nr_pts = 16;

	// Domain [0, 1] x [0, 1] x [0, 1]
	h = 1.0f / static_cast<float>(nr_pts - 1);

	// Right-hand side and exact solution of the poisson problem
	rhs.setZero(nr_pts*nr_pts*nr_pts);
	sol.setZero(nr_pts*nr_pts*nr_pts);
	for (Eigen::VectorXf::Index k = 0; k < static_cast<Eigen::VectorXf::Index>(nr_pts); k++)
	{
		for (Eigen::VectorXf::Index j = 0; j < static_cast<Eigen::VectorXf::Index>(nr_pts); j++)
		{
			for (Eigen::VectorXf::Index i = 0; i < static_cast<Eigen::VectorXf::Index>(nr_pts); i++)
			{
				float x = 0 + i * h;
				float y = 0 + j * h;
				float z = 0 + k * h;
				rhs(k*nr_pts*nr_pts + j*nr_pts + i) = 16 * 3.14159f*3.14159f*3.14159f * sin(2 * 3.14159f * x) * sin(2 * 3.14159f * y) * sin(2 * 3.14159f * z);
				sol(k*nr_pts*nr_pts + j*nr_pts + i) = sin(2 * 3.14159f * x) * sin(2 * 3.14159f * y) * sin(2 * 3.14159f * z);
			}
		}
	}

	return nr_pts;
}
template<typename Solver, typename Ctx, typename Dim>
void runPoissonTest(Dim dim, float h, Eigen::VectorXf& lhs, Eigen::VectorXf& rhs, const Eigen::VectorXf& sol, int max_iters, float eps)
{
	Eigen::Map<Eigen::VectorXf> x(lhs.data(), lhs.size());
	Eigen::Map<Eigen::VectorXf> y(rhs.data(), rhs.size());
	std::vector<unsigned char> invalid_cells(rhs.size(), 0);

	Ctx ctx{ dim };
	ctx.updatePoissonStencil(h, -1, { invalid_cells.data(), (int64_t)invalid_cells.size() });
	ctx.setData(&x, &y);

	// Execute the poisson solver
	Solver solver;
	solver.setMaxIterations(max_iters);
	solver.solve(&ctx);

	// Check for the solution
	Eigen::VectorXf err; err.setZero(sol.size());
	for (Eigen::VectorXf::Index i = 0; i < sol.size(); i++)
	{
		err(i) = fabs(lhs(i) - sol(i));
	}

	Eigen::VectorXf::Index max_err_idx;
	EXPECT_LE(err.maxCoeff(&max_err_idx), eps) << "Maximum error at index " << max_err_idx;
}