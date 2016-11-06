#pragma once

#include "variable.hpp"

#ifdef WITH_BLAS_SUPPORT

#include "tape.hpp"

#include <functional>
#include <vector>
#include <cmath>

#include <cblas.h>
#include <openvml.h>


namespace Bare
{
	namespace BLAS
	{
		template <typename DType>
		using Constant = Bare::Constant<DType>;

		template <typename DType>
		class Variable : public Bare::Variable<DType>
		{
			VARIABLE_INTERFACE(BLAS, Variable);
		};


		// Type specific methods
		template <typename DType>
		void matmul(Shape& shapeA, DType* a, Shape& shapeB, DType* b, DType* y, DType alpha = 1.0, DType beta = 1.0, bool transposeA = false, bool transposeB = false);

		template <> void matmul(Shape& shapeA, float* a, Shape& shapeB, float* b, float* y, float alpha, float beta, bool transposeA, bool transposeB)
		{
			auto shape_a = transposeA ? shapeA.T() : shapeA;
			auto shape_b = transposeB ? shapeB.T() : shapeB;

			int lda = transposeA ? shape_a.m : shape_a.n;
			int ldb = transposeB ? shape_a.n : shape_b.n;

			cblas_sgemm(CblasRowMajor, transposeA ? CblasTrans : CblasNoTrans, transposeB ? CblasTrans : CblasNoTrans,
				shape_a.m, shape_b.n, shape_a.n, alpha, a, lda, b, ldb, beta, y, shape_b.n);
		}

		template <> void matmul(Shape& shapeA, double* a, Shape& shapeB, double* b, double* y, double alpha, double beta, bool transposeA, bool transposeB)
		{
			auto shape_a = transposeA ? shapeA.T() : shapeA;
			auto shape_b = transposeB ? shapeB.T() : shapeB;

			int lda = transposeA ? shape_a.m : shape_a.n;
			int ldb = transposeB ? shape_a.n : shape_b.n;

			cblas_dgemm(CblasRowMajor, transposeA ? CblasTrans : CblasNoTrans, transposeB ? CblasTrans : CblasNoTrans,
				shape_a.m, shape_b.n, shape_a.n, alpha, a, lda, b, ldb, beta, y, shape_b.n);
		}

		template <typename DType>
		void elementwise_add(Shape& shape, DType* a, DType* b);

		template <> void elementwise_add(Shape& shape, float* a, float* b)
		{
		    vsAdd(shape.prod(), a, b, a);
		}

		template <> void elementwise_add(Shape& shape, double* a, double* b)
		{
		    vdAdd(shape.prod(), a, b, a);
		}

		template <typename DType>
		void elementwise_sub(Shape& shape, DType* a, DType* b);

		template <> void elementwise_sub(Shape& shape, float* a, float* b)
		{
		    vsSub(shape.prod(), a, b, a);
		}

		template <> void elementwise_sub(Shape& shape, double* a, double* b)
		{
		    vdSub(shape.prod(), a, b, a);
		}

		// c = alpha * a * b + beta * c
		template <typename DType>
		void elementwise_mul(Shape& shape, DType* a, DType* b, DType* c);

		template <> void elementwise_mul(Shape& shape, float* a, float* b, float* c)
		{
			vsAddmul(shape.prod(), a, b, c);
		}

		template <> void elementwise_mul(Shape& shape, double* a, double* b, double* c)
		{
			vdAddmul(shape.prod(), a, b, c);
		}

		// c = alpha * a * b + beta * c
		template <typename DType>
		void elementwise_div(Shape& shape, DType* a, DType* b, DType* c);

		template <> void elementwise_div(Shape& shape, float* a, float* b, float* c)
		{
			vsAdddiv(shape.prod(), a, b, c);
		}

		template <> void elementwise_div(Shape& shape, double* a, double* b, double* c)
		{
			vdAdddiv(shape.prod(), a, b, c);
		}

		// c = alpha * a * b + beta * c
		template <typename DType>
		void elementwise_pow(Shape& shape, DType* a, DType b, DType* c);

		template <> void elementwise_pow(Shape& shape, float* a, float b, float* c)
		{
			vsPowx(shape.prod(), a, &b, c);
		}

		template <> void elementwise_pow(Shape& shape, double* a, double b, double* c)
		{
			vdPowx(shape.prod(), a, &b, c);
		}


		template <typename DType>
		std::shared_ptr<Bare::BLAS::Variable<DType>> operator+(std::shared_ptr<Bare::BLAS::Variable<DType>> a, std::shared_ptr<Bare::BLAS::Variable<DType>> b)
		{
			auto r = std::make_shared<Bare::BLAS::Variable<DType>>(a, 0);

		    // Sum
			elementwise_add(a->shape(), r->values(), b->values());

		    // Adjoint
			Tape::current()->push([a, b, r]() {
		        elementwise_add(a->shape(), a->adjoints(), r->adjoints());
		        elementwise_add(b->shape(), b->adjoints(), r->adjoints());
			});

			return r;
		}

		template <typename DType>
		std::shared_ptr<Bare::BLAS::Variable<DType>> operator-(std::shared_ptr<Bare::BLAS::Variable<DType>> a, std::shared_ptr<Bare::BLAS::Variable<DType>> b)
		{
			auto r = std::make_shared<Bare::BLAS::Variable<DType>>(a, 0);

		    // Substract
		    elementwise_sub(a->shape(), r->values(), b->values());

		    // Adjoint
			Tape::current()->push([a, b, r]() {
		        elementwise_add(a->shape(), a->adjoints(), r->adjoints());
		        elementwise_sub(b->shape(), b->adjoints(), r->adjoints());
			});

			return r;
		}

		template <typename DType>
		std::shared_ptr<Bare::BLAS::Variable<DType>> operator*(std::shared_ptr<Bare::BLAS::Variable<DType>> a, std::shared_ptr<Bare::BLAS::Variable<DType>> b)
		{
			auto r = std::make_shared<Bare::BLAS::Variable<DType>>(a->shape(), 0);
		    elementwise_mul(a->shape(), a->values(), b->values(), r->_values);

			Tape::current()->push([a, b, r]() {
				elementwise_mul(a->shape(), b->values(), r->adjoints(), a->adjoints());
				elementwise_mul(a->shape(), a->values(), r->adjoints(), b->adjoints());
			});

			return r;
		}

		template <typename DType>
		std::shared_ptr<Bare::BLAS::Variable<DType>> operator/(std::shared_ptr<Bare::BLAS::Variable<DType>> a, std::shared_ptr<Bare::BLAS::Variable<DType>> b)
		{
			auto r = std::make_shared<Bare::BLAS::Variable<DType>>(a->shape(), 0);
		    elementwise_div(a->shape(), a->values(), b->values(), r->values());

			Tape::current()->push([a, b, r]() {
				// TODO(gpascualg): SIMD parallelization
				SHAPE_LOOP(a->shape()) {
					auto bv2 = b->values(x, y) * b->values(x, y);

					a->adjoints(x, y) += b->values(x, y) * r->adjoints(x, y) / bv2;
					b->adjoints(x, y) -= a->values(x, y) * r->adjoints(x, y) / bv2;
				}
			});

			return r;
		}

		template <typename DType>
		std::shared_ptr<Bare::BLAS::Variable<DType>> operator/(std::shared_ptr<Bare::BLAS::Variable<DType>> a, std::shared_ptr<Constant<DType>> b)
		{
			SHAPE_LOOP(a->shape())
			{
				a->values(x, y) /= b->value();
			}

			Tape::current()->push([a, b]() {
				// TODO(gpascualg): SIMD parallelization
				SHAPE_LOOP(a->shape()) {
					a->adjoints(x, y) /= b->value();
				}
			});

			return a;
		}

		template <typename DType>
		void cblas_transpose(std::shared_ptr<Bare::BLAS::Variable<DType>> a, std::shared_ptr<Bare::BLAS::Variable<DType>> r);

		template <>
		void cblas_transpose(std::shared_ptr<Bare::BLAS::Variable<float>> a, std::shared_ptr<Bare::BLAS::Variable<float>> r)
		{
			cblas_somatcopy(CblasRowMajor, CblasTrans, a->shape().m, a->shape().n, 1.0, a->values(), a->shape().n, r->values(), a->shape().m);
		}

		template <>
		void cblas_transpose(std::shared_ptr<Bare::BLAS::Variable<double>> a, std::shared_ptr<Bare::BLAS::Variable<double>> r)
		{
			cblas_domatcopy(CblasRowMajor, CblasTrans, a->shape().m, a->shape().n, 1.0, a->values(), a->shape().n, r->values(), a->shape().m);
		}

		template <typename DType>
		std::shared_ptr<Bare::BLAS::Variable<DType>> sum(std::shared_ptr<Bare::BLAS::Variable<DType>> a)
		{
			auto r = std::make_shared<Bare::BLAS::Variable<DType>>(Shape {1, 1}, 0);

			// TODO(gpascual): SIMD
			SHAPE_LOOP(a->shape()) {
				r->values(0, 0) += a->values(x, y);
			}

			Tape::current()->push([a, r]() {
				SHAPE_LOOP(a->shape()) {
					a->adjoints(x, y) += r->adjoints(0, 0);
				}
			});

			return r;
		}

		template <typename DType>
		std::shared_ptr<Bare::BLAS::Variable<DType>> transpose(std::shared_ptr<Bare::BLAS::Variable<DType>> a)
		{
			auto r = std::make_shared<Bare::BLAS::Variable<DType>>(a->shape().T());
			// TODO(gpascualg): Test the implementation below
			//cblas_transpose(a, r);

			SHAPE_LOOP(a->shape()) {
				r->values(y, x) = a->values(x, y);
			}

			Tape::current()->push([a, r]() {
				elementwise_add(a->shape(), a->adjoints(), r->adjoints());
			});

			return r;
		}

		template <typename DType>
		std::shared_ptr<Bare::BLAS::Variable<DType>> sqrt(std::shared_ptr<Bare::BLAS::Variable<DType>> a)
		{
			auto r = std::make_shared<Bare::BLAS::Variable<DType>>(a->shape(), 0);
			// TODO(gpascual): Implement vsSqr
			// TODO(gpascualg): Why does calling this from pyhon crash??
			//elementwise_pow(a->shape(), a->values(), DType(0.5), r->_values);
			SHAPE_LOOP(a->shape()) {
				r->values(x, y) += std::sqrt(a->values(x, y));
			}

			Tape::current()->push([r, a]() {
				// TODO(gpascual): SIMD implementation
				SHAPE_LOOP(a->shape()) {
					a->adjoints(x, y) = r->adjoints(x, y) / (DType(2.0) * r->values(x, y) + 1e-6);
				}
			});

			return r;
		}

		template <typename DType, typename D>
		std::shared_ptr<Bare::BLAS::Variable<DType>> pow(std::shared_ptr<Bare::BLAS::Variable<DType>> a, D expo)
		{
			auto r = std::make_shared<Bare::BLAS::Variable<DType>>(a->shape(), 0);
			//elementwise_pow(a->shape(), a->values(), DType(expo), r->_values);
			// TODO(gpascualg): Why does calling this from pyhon crash??
			SHAPE_LOOP(a->shape()) {
				r->values(x, y) += std::pow(a->values(x, y), expo);
			}

			Tape::current()->push([a, r, expo]() {
				SHAPE_LOOP(a->shape()) {
					a->adjoints(x, y) += r->adjoints(x, y) * expo * std::pow(a->values(x, y), DType(expo - 1));
				}
			});

			return r;
		}

		template <typename DType>
		std::shared_ptr<Bare::BLAS::Variable<DType>> mul(std::shared_ptr<Bare::BLAS::Variable<DType>> a, std::shared_ptr<Bare::BLAS::Variable<DType>> b)
		{
			auto r = std::make_shared<Bare::BLAS::Variable<DType>>(Shape {a->shape().m, b->shape().n}, 0);
			matmul(a->shape(), a->values(), b->shape(), b->values(), r->values());

			Tape::current()->push([a, b, r]{
				matmul(r->shape(), r->adjoints(), b->shape(), b->values(), a->adjoints(), DType(1.0), DType(1.0), false, true);
				matmul(a->shape(), a->values(), r->shape(), r->adjoints(), b->adjoints(), DType(1.0), DType(1.0), true, false);
			});

			return r;
		}

		template <typename DType>
		std::shared_ptr<Bare::BLAS::Variable<DType>> slice(std::shared_ptr<Bare::BLAS::Variable<DType>> a, int x0, int y0, int dx, int dy)
		{
			return std::make_shared<Bare::BLAS::Variable<DType>>(a, x0, y0, dx, dy);
		}
	}
}

namespace F32
{
	namespace BLAS
	{
		inline std::shared_ptr<Bare::BLAS::Variable<float>> Variable(Shape shape, float value)
		{
			return make_variable<Bare::BLAS::Variable<float>>(shape, value);
		}
	}
}

namespace F64
{
	namespace BLAS
	{
		inline std::shared_ptr<Bare::BLAS::Variable<double>> Variable(Shape shape, double value)
		{
			return make_variable<Bare::BLAS::Variable<double>>(shape, value);
		}
	}
}

#endif
