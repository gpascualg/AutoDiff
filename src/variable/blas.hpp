#pragma once

#include "variable.hpp"

#ifdef WITH_BLAS_SUPPORT

#include "blas_i.hpp"
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
			using Bare::Variable<DType>::Variable;

			ADD_FRIENDS(Bare::BLAS::Variable)

		protected:
			SharedVariable<DType> add(SharedVariable<DType> a, SharedVariable<DType> b) override
			{
				auto r = _add(a, b);

			    // Adjoint
				Tape::current()->push([a, b, r]() {
			        blas_add(a->shape(), a->adjoints(), r->adjoints());
			        blas_add(b->shape(), b->adjoints(), r->adjoints());
				});

				return r;
			}

			SharedVariable<DType> sub(SharedVariable<DType> a, SharedVariable<DType> b) override
			{
				auto r = _sub(a, b);

			    // Adjoint
				Tape::current()->push([a, b, r]() {
			        blas_add(a->shape(), a->adjoints(), r->adjoints());
			        blas_sub(b->shape(), b->adjoints(), r->adjoints());
				});

				return r;
			}

			SharedVariable<DType> elementwise_mul(SharedVariable<DType> a, SharedVariable<DType> b) override
			{
				auto r = _elementwise_mul(a, b);

				Tape::current()->push([a, b, r]() {
					blas_mul(a->shape(), b->values(), r->adjoints(), a->adjoints());
					blas_mul(a->shape(), a->values(), r->adjoints(), b->adjoints());
				});

				return r;
			}

			SharedVariable<DType> elementwise_mul(SharedVariable<DType> a, SharedConstant<DType> b) override
			{
				a = _elementwise_mul(a, b);

				Tape::current()->push([a, b]() {
					// TODO(gpascualg): SIMD parallelization
					SHAPE_LOOP(a->shape()) {
						a->adjoints(x, y) *= b->value();
					}
				});

				return a;
			}

			SharedVariable<DType> elementwise_div(SharedVariable<DType> a, SharedVariable<DType> b) override
			{
				auto r = _elementwise_div(a, b);

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

			SharedVariable<DType> elementwise_div(SharedVariable<DType> a, SharedConstant<DType> b) override
			{
				a = _elementwise_div(a, b);

				Tape::current()->push([a, b]() {
					// TODO(gpascualg): SIMD parallelization
					SHAPE_LOOP(a->shape()) {
						a->adjoints(x, y) /= b->value();
					}
				});

				return a;
			}

			SharedVariable<DType> sum(SharedVariable<DType> a) override
			{
				auto r = _sum(a);

				Tape::current()->push([a, r]() {
					SHAPE_LOOP(a->shape()) {
						a->adjoints(x, y) += r->adjoints(0, 0);
					}
				});

				return r;
			}

			SharedVariable<DType> sqrt(SharedVariable<DType> a) override
			{
				auto r = _sqrt(a);

				Tape::current()->push([r, a]() {
					// TODO(gpascual): SIMD implementation
					SHAPE_LOOP(a->shape()) {
						a->adjoints(x, y) = r->adjoints(x, y) / (DType(2.0) * r->values(x, y) + 1e-6);
					}
				});

				return r;
			}

			SharedVariable<DType> pow(SharedVariable<DType> a, float expo) override
			{
				auto r = _pow(a, expo);

				Tape::current()->push([a, r, expo]() {
					SHAPE_LOOP(a->shape()) {
						a->adjoints(x, y) += r->adjoints(x, y) * expo * std::pow(a->values(x, y), DType(expo - 1));
					}
				});

				return r;
			}

			SharedVariable<DType> mul(SharedVariable<DType> a, SharedVariable<DType> b) override
			{
				auto r = _mul(a, b);

				Tape::current()->push([a, b, r]{
					blas_matmul(r->shape(), r->adjoints(), b->shape(), b->values(), a->adjoints(), DType(1.0), DType(1.0), false, true);
					blas_matmul(a->shape(), a->values(), r->shape(), r->adjoints(), b->adjoints(), DType(1.0), DType(1.0), true, false);
				});

				return r;
			}

			SharedVariable<DType> transpose(SharedVariable<DType> a) override
			{
				auto r = _transpose(a);

				Tape::current()->push([a, r]() {
					blas_add(a->shape(), a->adjoints(), r->adjoints());
				});

				return r;
			}

			SharedVariable<DType> slice(SharedVariable<DType> a, int x0, int y0, int dx, int dy)
			{
				return std::make_shared<Bare::BLAS::Variable<DType>>(a, x0, y0, dx, dy);
			}

			SharedVariable<DType> _add(SharedVariable<DType> a, SharedVariable<DType> b) override
			{
				auto r = std::make_shared<Bare::BLAS::Variable<DType>>(a, 0);
				blas_add(a->shape(), r->values(), b->values());
				return r;
			}

			SharedVariable<DType> _sub(SharedVariable<DType> a, SharedVariable<DType> b) override
			{
				auto r = std::make_shared<Bare::BLAS::Variable<DType>>(a, 0);
				blas_sub(a->shape(), r->values(), b->values());
				return r;
			}

			SharedVariable<DType> _elementwise_mul(SharedVariable<DType> a, SharedVariable<DType> b) override
			{
				auto r = std::make_shared<Bare::BLAS::Variable<DType>>(a->shape(), 0);
			    blas_mul(a->shape(), a->values(), b->values(), r->_values);
				return r;
			}

			SharedVariable<DType> _elementwise_mul(SharedVariable<DType> a, SharedConstant<DType> b) override
			{
				SHAPE_LOOP(a->shape())
				{
					a->values(x, y) *= b->value();
				}

				return a;
			}

			SharedVariable<DType> _elementwise_div(SharedVariable<DType> a, SharedVariable<DType> b) override
			{
				auto r = std::make_shared<Bare::BLAS::Variable<DType>>(a->shape(), 0);
			    blas_div(a->shape(), a->values(), b->values(), r->values());
				return r;
			}

			SharedVariable<DType> _elementwise_div(SharedVariable<DType> a, SharedConstant<DType> b) override
			{
				SHAPE_LOOP(a->shape())
				{
					a->values(x, y) /= b->value();
				}

				return a;
			}

			SharedVariable<DType> _sum(SharedVariable<DType> a) override
			{
				auto r = std::make_shared<Bare::BLAS::Variable<DType>>(Shape {1, 1}, 0);

				// TODO(gpascual): SIMD
				SHAPE_LOOP(a->shape()) {
					r->values(0, 0) += a->values(x, y);
				}

				return r;
			}

			SharedVariable<DType> _sqrt(SharedVariable<DType> a) override
			{
				auto r = std::make_shared<Bare::BLAS::Variable<DType>>(a->shape(), 0);
				// TODO(gpascual): Implement vsSqr
				// TODO(gpascualg): Why does calling this from pyhon crash??
				//elementwise_pow(a->shape(), a->values(), DType(0.5), r->_values);
				SHAPE_LOOP(a->shape()) {
					r->values(x, y) += std::sqrt(a->values(x, y));
				}

				return r;
			}

			SharedVariable<DType> _pow(SharedVariable<DType> a, float expo) override
			{
				auto r = std::make_shared<Bare::BLAS::Variable<DType>>(a->shape(), 0);
				//elementwise_pow(a->shape(), a->values(), DType(expo), r->_values);
				// TODO(gpascualg): Why does calling this from pyhon crash??
				SHAPE_LOOP(a->shape()) {
					r->values(x, y) += std::pow(a->values(x, y), expo);
				}

				return r;
			}

			SharedVariable<DType> _mul(SharedVariable<DType> a, SharedVariable<DType> b) override
			{
				auto r = std::make_shared<Bare::BLAS::Variable<DType>>(Shape {a->shape().m, b->shape().n}, 0);
				blas_matmul(a->shape(), a->values(), b->shape(), b->values(), r->values());
				return r;
			}

			SharedVariable<DType> _transpose(SharedVariable<DType> a) override
			{
				auto r = std::make_shared<Bare::BLAS::Variable<DType>>(a->shape().T());
				// TODO(gpascualg): parallelization
				SHAPE_LOOP(a->shape()) {
					r->values(y, x) = a->values(x, y);
				}

				return r;
			}
		};

		DECLARE_FRIENDS(Bare::BLAS::Variable);
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
