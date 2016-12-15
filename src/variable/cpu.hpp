#pragma once

#include "variable.hpp"

#ifndef SKIP_BARE_CPU

#include "tape.hpp"
#include "ops/cpu.hpp"

#include <functional>
#include <vector>
#include <cmath>

namespace Bare
{
	namespace CPU
	{
		template <typename DType>
		using Constant = Bare::Constant<DType>;

		template <typename DType>
		class Variable : public Bare::Variable<DType>
		{
			template <typename T> friend class ::Optimizer;
			using Bare::Variable<DType>::Variable;

		protected:
			template <typename T>
			inline std::shared_ptr<Bare::CPU::Variable<DType>> cast(std::shared_ptr<T> p)
			{
				return std::dynamic_pointer_cast<Bare::CPU::Variable<DType>>(p);
			}

			template <typename T>
			inline std::shared_ptr<SpecializedTapeVariable<DType>> downcast(std::shared_ptr<T> p)
			{
				return std::dynamic_pointer_cast<SpecializedTapeVariable<DType>>(p);
			}

			template <typename T>
			friend std::shared_ptr<Bare::CPU::Variable<T>> operator+(std::shared_ptr<Bare::CPU::Variable<T>> a, std::shared_ptr<Bare::CPU::Variable<T>> b);

			/*
			SharedVariable<DType> add(SharedVariable<DType> a, SharedVariable<DType> b) override
			{
				auto r = _add(a, b);

				Tape::current()->push([&a, &b, r]() {
					SHAPE_LOOP(a->shape()) {
						a->adjoints(x, y) += r->adjoints(x, y);
						b->adjoints(x, y) += r->adjoints(x, y);
					}
				});

				return cast(r);
			}

			SharedVariable<DType> sub(SharedVariable<DType> a, SharedVariable<DType> b) override
			{
				auto r = _sub(a, b);

			    // Adjoint
				Tape::current()->push([&a, &b, r]() {
					SHAPE_LOOP(a->shape()) {
						a->adjoints(x, y) += r->adjoints(x, y);
						b->adjoints(x, y) -= r->adjoints(x, y);
					}
				});

				return cast(r);
			}

			SharedVariable<DType> elementwise_mul(SharedVariable<DType> a, SharedVariable<DType> b) override
			{
				auto r = _elementwise_mul(a, b);

				Tape::current()->push([a, b, r]() {
					SHAPE_LOOP(a->shape())
					{
						a->adjoints(x, y) += b->values(x, y) * r->adjoints(x, y);
						b->adjoints(x, y) += a->values(x, y) * r->adjoints(x, y);
					}
				});

				return cast(r);
			}

			SharedVariable<DType> elementwise_mul(SharedVariable<DType> a, SharedConstant<DType> b) override
			{
				auto r = _elementwise_mul(a, b);

				Tape::current()->push([r, b]() {
					SHAPE_LOOP(r->shape()) {
						r->adjoints(x, y) *= b->values(0, 0);
					}
				});

				return cast(r);
			}

			SharedVariable<DType> elementwise_div(SharedVariable<DType> a, SharedVariable<DType> b) override
			{
				auto r = _elementwise_div(a, b);

				Tape::current()->push([a, b, r]() {
					SHAPE_LOOP(a->shape()) {
						auto bv2 = b->values(x, y) * b->values(x, y);

						a->adjoints(x, y) += b->values(x, y) * r->adjoints(x, y) / bv2;
						b->adjoints(x, y) -= a->values(x, y) * r->adjoints(x, y) / bv2;
					}
				});

				return cast(r);
			}

			SharedVariable<DType> elementwise_div(SharedVariable<DType> a, SharedConstant<DType> b) override
			{
				auto r = _elementwise_div(a, b);

				Tape::current()->push([r, b]() {
					SHAPE_LOOP(r->shape()) {
						r->adjoints(x, y) /= b->values(0, 0);
					}
				});

				return cast(r);
			}

			SharedVariable<DType> sum(SharedVariable<DType> a) override
			{
				auto r = _sum(a);

				Tape::current()->push([a, r]() {
					SHAPE_LOOP(a->shape()) {
						a->adjoints(x, y) += r->adjoints(0, 0);
					}
				});

				return cast(r);
			}

			SharedVariable<DType> sqrt(SharedVariable<DType> a) override
			{
				auto r = _sqrt(a);

				Tape::current()->push([r, a]() {
					SHAPE_LOOP(a->shape()) {
						a->adjoints(x, y) = r->adjoints(x, y) / (DType(2.0) * r->values(x, y) + 1e-6);
					}
				});

				return cast(r);
			}

			SharedVariable<DType> pow(SharedVariable<DType> a, float expo) override
			{
				auto r = _pow(a, expo);

				Tape::current()->push([a, r, expo]() {
					SHAPE_LOOP(a->shape()) {
						a->adjoints(x, y) += r->adjoints(x, y) * expo * std::pow(a->values(x, y), DType(expo - 1));
					}
				});

				return cast(r);
			}

			SharedVariable<DType> mul(SharedVariable<DType> a, SharedVariable<DType> b) override
			{
				auto r = _mul(a, b);

				Tape::current()->push([a, b, r]{
					for (int i = 0; i < a->shape().m; ++i)
					{
						for (int j = 0; j < a->shape().n; ++j)
						{
							for (int k = 0; k < r->shape().n; ++k)
							{
								a->adjoints(i, j) += r->adjoints(i, k) * b->values(k, j);
							}
						}
					}

					for (int i = 0; i < b->shape().m; ++i)
					{
						for (int j = 0; j < b->shape().n; ++j)
						{
							for (int k = 0; k < a->shape().n; ++k)
							{
								b->adjoints(i, j) += a->values(i, k) * r->adjoints(k, j);
							}
						}
					}
				});

				return cast(r);
			}

			SharedVariable<DType> transpose(SharedVariable<DType> a) override
			{
				auto r = _transpose(a);

				Tape::current()->push([a, r]() {
					SHAPE_LOOP(a->shape())
					{
						a->adjoints(x, y) += r->adjoints(x, y);
					}
				});

				return cast(r);
			}

			SharedVariable<DType> slice(SharedVariable<DType> a, int x0, int y0, int dx, int dy)
			{
				return std::make_shared<Bare::CPU::Variable<DType>>(a, x0, y0, dx, dy);
			}
			*/
		};

		template <typename DType>
		std::shared_ptr<Bare::CPU::Variable<DType>> operator+(std::shared_ptr<Bare::CPU::Variable<DType>> a, std::shared_ptr<Bare::CPU::Variable<DType>> b)
		{
			auto r = CPUOps<DType>::get()->add(Tape::current(), a->shape(), a->_values, b->_values);
			return std::make_shared<Bare::CPU::Variable<DType>>(CPUOps<DType>::get(), a->shape(), r);
		}
	}
}

namespace F32
{
	namespace CPU
	{
		inline std::shared_ptr<Bare::CPU::Variable<float>> Variable(Shape shape, float value)
		{
			return make_variable<Bare::CPU::Variable<float>>(CPUOps<float>::get(), shape, value);
		}
	}
}

namespace F64
{
	namespace CPU
	{
		inline std::shared_ptr<Bare::CPU::Variable<double>> Variable(Shape shape, double value)
		{
			return make_variable<Bare::CPU::Variable<double>>(CPUOps<double>::get(), shape, value);
		}
	}
}

#endif
