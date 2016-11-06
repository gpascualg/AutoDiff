#pragma once

#ifdef WITH_BARE_CPU

#include "variable.hpp"
#include "tape.hpp"

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
			VARIABLE_INTERFACE(CPU, Variable)
		};


		template <typename DType>
		CPU::Variable<DType>& operator+(CPU::Variable<DType>& a, CPU::Variable<DType>& b)
		{
			auto r = new CPU::Variable<DType>(a.shape());

			// Do the sum
			SHAPE_LOOP(a.shape()) {
				r->values(x, y) = a.vales(x, y) + b.values(x, y);
			}

			Tape::current()->push([&a, &b, r]() {
				SHAPE_LOOP(a.shape()) {
					a.adjoints(x, y) += r->adjoints(x, y);
					b.adjoints(x, y) += r->adjoints(x, y);
				}
			});

			return *r;
		}

		template <typename DType>
		CPU::Variable<DType>& operator-(CPU::Variable<DType>& a, CPU::Variable<DType>& b)
		{
			auto r = new CPU::Variable<DType>(a.shape());

			// Do the sum
			SHAPE_LOOP(a.shape()) {
				r->values(x, y) = a.values(x, y) - b.values(x, y);
			}

			Tape::current()->push([&a, &b, r]() {
				SHAPE_LOOP(a.shape()) {
					a.adjoints(x, y) += r->adjoints(x, y);
					b.adjoints(x, y) -= r->adjoints(x, y);
				}
			});

			return *r;
		}

		template <typename DType>
		CPU::Variable<DType>& operator*(CPU::Variable<DType>& a, CPU::Variable<DType>& b)
		{
			auto r = new CPU::Variable<DType>(a.shape());

			// Do the sum
			SHAPE_LOOP(a.shape()) {
				r->values(x, y) = a.values(x, y) * b.values(x, y);
			}

			DType* av = a.values();
			DType* bv = b.values();

			Tape::current()->push([&a, &b, r]() {
				SHAPE_LOOP(a.shape()) {
					a.adjoints(x, y) += b.values(x, y) * r->adjoints(x, y);
					b.adjoints(x, y) += a.values(x, y) * r->adjoints(x, y);
				}
			});

			return *r;
		}

		template <typename DType>
		CPU::Variable<DType>& operator/(CPU::Variable<DType>& a, CPU::Variable<DType>& b)
		{
			auto r = new CPU::Variable<DType>(a.shape());

			// Do the sum
			SHAPE_LOOP(a.shape()) {
				r->values(x, y) = a.values(x, y) / b.values(x, y);
			}

			Tape::current()->push([&a, &b, r]() {
				SHAPE_LOOP(a.shape()) {
					auto bv2 = b.values(x, y) * b.values(x, y);

					a.adjoints(x, y) += b.values(x, y) * r->adjoints(x, y) / bv2;
					b.adjoints(x, y) -= a.values(x, y) * r->adjoints(x, y) / bv2;
				}
			});

			return *r;
		}

		template <typename DType>
		CPU::Variable<DType>& sqrt(CPU::Variable<DType>& a)
		{
			auto r = new CPU::Variable<DType>(a.shape());

			// Do the sum
			SHAPE_LOOP(a.shape()) {
				r->values(x, y) = std::sqrt(a.values(x, y));
			}

			DType* rv = r->_values;
			DType* adjs = r->_adjs;

			Tape::current()->push([&a, r]() {
				SHAPE_LOOP(a.shape()) {
					a.adjoints(x, y) += r->adjoints(x, y) / (DType(2.0) * r->values(x, y));
				}
			});

			return *r;
		}

		template <typename DType, typename D>
		CPU::Variable<DType>& pow(CPU::Variable<DType>& a, D expo)
		{
			auto r = new CPU::Variable<DType>(a.shape());

			// Do the sum
			SHAPE_LOOP(a.shape()) {
				r->values(x, y) = std::pow(a->values(x, y), expo);
			}

			DType* rv = r->_values;
			DType* adjs = r->_adjs;
			DType* av = a._values;

			Tape::current()->push([&a, r, expo]() {
				SHAPE_LOOP(a.shape()) {
					a->adjoints(x, y) += r->adjoints(x, y) * expo * std::pow(a.values(x, y), DType(expo - 1));
				}
			});

			return *r;
		}
	}
}

namespace F32
{
	namespace CPU
	{
		inline Bare::CPU::Variable<float>& Variable(Shape shape, float value)
		{
			return make_variable<Bare::CPU::Variable<float>>(shape, value);
		}
	}
}

namespace F64
{
	namespace CPU
	{
		inline Bare::CPU::Variable<double>& Variable(Shape shape, double value)
		{
			return make_variable<Bare::CPU::Variable<double>>(shape, value);
		}
	}
}

#endif
