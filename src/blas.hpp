#ifdef WITH_BLAS_SUPPORT

#pragma once

#include "variable.hpp"
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
		class Variable : public Bare::Variable<DType>
		{
			VARIABLE_INTERFACE(BLAS, Variable)
		};


        // Type specific methods
        template <typename DType>
        void matmul(Shape& shapeA, DType* a, Shape& shapeB, DType* b, DType* y, DType alpha = 1.0, DType beta = 1.0, bool transposeA = false, bool transposeB = false);

        template <> void matmul(Shape& shapeA, float* a, Shape& shapeB, float* b, float* y, float alpha, float beta, bool transposeA, bool transposeB)
        {
			int lda = transposeA ? shapeA.m : shapeA.n;
			int ldb = transposeB ? shapeA.n : shapeB.n;

			cblas_sgemm(CblasRowMajor, transposeA ? CblasTrans : CblasNoTrans, transposeB ? CblasTrans : CblasNoTrans,
				shapeA.m, shapeB.n, shapeA.n, alpha, a, lda, b, ldb, beta, y, shapeB.n);
        }

        template <> void matmul(Shape& shapeA, double* a, Shape& shapeB, double* b, double* y, double alpha, double beta, bool transposeA, bool transposeB)
        {
			int lda = transposeA ? shapeA.m : shapeA.n;
			int ldb = transposeB ? shapeA.n : shapeB.n;

			cblas_dgemm(CblasRowMajor, transposeA ? CblasTrans : CblasNoTrans, transposeB ? CblasTrans : CblasNoTrans,
				shapeA.m, shapeB.n, shapeA.n, alpha, a, lda, b, ldb, beta, y, shapeB.n);
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
		BLAS::Variable<DType>& operator+(BLAS::Variable<DType>& a, BLAS::Variable<DType>& b)
		{
			auto r = new BLAS::Variable<DType>(a);

            // Sum
			elementwise_add(a.shape(), r->values(), b.values());

            // Adjoint
			Tape::current()->push([&a, &b, r]() {
                elementwise_add(a.shape(), a.adjoints(), r->_adjs);
                elementwise_add(b.shape(), b.adjoints(), r->_adjs);
			});

			return *r;
		}

		template <typename DType>
		BLAS::Variable<DType>& operator-(BLAS::Variable<DType>& a, BLAS::Variable<DType>& b)
		{
			auto r = new BLAS::Variable<DType>(a);

            // Substract
            elementwise_sub(a.shape(), r->values(), b.values());

            // Adjoint
			Tape::current()->push([&a, &b, r]() {
                elementwise_add(a.shape(), a.adjoints(), r->_adjs);
                elementwise_sub(b.shape(), b.adjoints(), r->_adjs);
			});

			return *r;
		}

		template <typename DType>
		BLAS::Variable<DType>& operator*(BLAS::Variable<DType>& a, BLAS::Variable<DType>& b)
		{
			auto r = new BLAS::Variable<DType>(a.shape());
            elementwise_mul(a.shape(), a.values(), b.values(), r->_values);

			Tape::current()->push([&a, &b, r]() {
				elementwise_mul(a.shape(), b.values(), r->adjoints(), a.adjoints());
				elementwise_mul(a.shape(), a.values(), r->adjoints(), b.adjoints());
			});

			return *r;
		}

		template <typename DType>
		BLAS::Variable<DType>& operator/(BLAS::Variable<DType>& a, BLAS::Variable<DType>& b)
		{
			auto r = new BLAS::Variable<DType>(a.shape());
            elementwise_div(a.shape(), a.values(), b.values(), r->values());

			Tape::current()->push([&a, &b, r]() {
				// TODO(gpascualg): SIMD parallelization
				SHAPE_LOOP(a.shape()) {
					auto bv2 = b.values(x, y) * b.values(x, y);

					a.adjoints(x, y) += b.values(x, y) * r->adjoints(x, y) / bv2;
					b.adjoints(x, y) -= a.values(x, y) * r->adjoints(x, y) / bv2;
				}
			});

			return *r;
		}

		template <typename DType>
		BLAS::Variable<DType>& transpose(BLAS::Variable<DType>& a)
		{
			auto r = new BLAS::Variable<DType>(a.shape().T());
			cblas_somatcopy(CblasRowMajor, CblasTrans, a.shape().m, a.shape().n, 1.0, a.values(), a.shape().n, r->_values, a.shape().m);

			Tape::current()->push([&a, r]() {
				elementwise_add(a.shape(), a.adjoints(), r->adjoints());
			});

			return *r;
		}

		template <typename DType>
		BLAS::Variable<DType>& sqrt(BLAS::Variable<DType>& a)
		{
			auto r = new BLAS::Variable<DType>(a.shape());
			// TODO(gpascual): Implement vsSqr
			elementwise_pow(a.shape(), a.values(), 0.5, r->_values);

			DType* rv = r->_values;
			DType* adjs = r->_adjs;

			Tape::current()->push([r, &a]() {
				// TODO(gpascual): SIMD implementation
				SHAPE_LOOP(a.shape()) {
					a.adjoints(x, y) = r->adjoints(x, y) / (DType(2.0) * r->values(x, y));
				}
			});

			return *r;
		}

		template <typename DType, typename D>
		BLAS::Variable<DType>& pow(BLAS::Variable<DType>& a, D expo)
		{
			auto r = new BLAS::Variable<DType>(a.shape());
			elementwise_pow(a.shape(), a.values(), (DType)expo, r->_values);

			Tape::current()->push([&a, r, expo]() {
				SHAPE_LOOP(a.shape()) {
					a.adjoints(x, y) += r->adjoints(x, y) * expo * std::pow(a.values(x, y), DType(expo - 1));
				}
			});

			return *r;
		}

		template <typename DType>
		BLAS::Variable<DType>& mul(BLAS::Variable<DType>& a, BLAS::Variable<DType>& b)
		{
			auto r = new BLAS::Variable<DType>({a.shape().n, b.shape().m});
			matmul(a.shape(), a.values(), b.shape(), b.values(), r->_values);

			Tape::current()->push([&a, &b, r]{
				matmul(r->shape(), r->adjoints(), b.shape(), b.values(), a.adjoints(), DType(1.0), DType(1.0), false, true);
				matmul(a.shape(), a.values(), r->shape(), r->adjoints(), b.adjoints(), DType(1.0), DType(1.0), true, false);
			});

			return *r;
		}

		template <typename DType>
		BLAS::Variable<DType>& slice(BLAS::Variable<DType>& a, int x0, int y0, int dx, int dy)
		{
			auto r = new BLAS::Variable<DType>(a, x0, y0, dx, dy);
			return *r;
		}
	}
}

namespace F32
{
	namespace BLAS
	{
		inline Bare::BLAS::Variable<float>& Variable(Shape shape, float value)
		{
			return *(new Bare::BLAS::Variable<float>(shape, value));
		}
	}
}

namespace F64
{
	namespace BLAS
	{
		inline Bare::BLAS::Variable<double>& Variable(Shape shape, double value)
		{
			return *(new Bare::BLAS::Variable<double>(shape, value));
		}
	}
}

#endif
