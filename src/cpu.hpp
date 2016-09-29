#pragma once

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
		class Variable : public Bare::Variable<DType>
		{
			template <typename T> friend CPU::Variable<T>& operator+(CPU::Variable<T>& a, CPU::Variable<T>& b);
			template <typename T> friend CPU::Variable<T>& operator-(CPU::Variable<T>& a, CPU::Variable<T>& b);
			template <typename T> friend CPU::Variable<T>& operator*(CPU::Variable<T>& a, CPU::Variable<T>& b);
			template <typename T> friend CPU::Variable<T>& operator/(CPU::Variable<T>& a, CPU::Variable<T>& b);
			template <typename T> friend CPU::Variable<T>& sqrt(CPU::Variable<T>& a);
			template <typename T, typename D> friend CPU::Variable<T>& pow(CPU::Variable<T>& a, D expo);

		public:
			Variable(Shape shape, DType value = 0, DType adj = 0) :
				Bare::Variable<DType>(shape)
			{
				_values = new DType[shape.prod()];
				_adjs = new DType[shape.prod()];

				SHAPE_LOOP(shape) {
					_values[shape.idx(x, y)] = value;
					_adjs[shape.idx(x, y)] = adj;
				}
			}

			virtual ~Variable()
			{
				delete[] _values;
				delete[] _adjs;
			}

			DType* operator()() override { return _values; }
			DType* adj() override { return _adjs; }

			void flag() override
			{
				// Set all adjacent values to 1
				SHAPE_LOOP(this->_shape) {
					_adjs[this->_shape.idx(x, y)] = 1;
				}
			}

		protected:
			DType* _values;
			DType* _adjs;
		};


		template <typename DType>
		CPU::Variable<DType>& operator+(CPU::Variable<DType>& a, CPU::Variable<DType>& b)
		{
			auto r = new CPU::Variable<DType>(a._shape);

			// Do the sum
			SHAPE_LOOP(a._shape) {
				r->_values[a._shape.idx(x, y)] = a._values[a._shape.idx(x, y)] + b._values[a._shape.idx(x, y)];
			}

			Tape::current()->push([&a, &b, r]() {
				SHAPE_LOOP(a._shape) {
					a._adjs[a._shape.idx(x, y)] += r->_adjs[a._shape.idx(x, y)];
					b._adjs[a._shape.idx(x, y)] += r->_adjs[a._shape.idx(x, y)];
				}
			});

			return *r;
		}

		template <typename DType>
		CPU::Variable<DType>& operator-(CPU::Variable<DType>& a, CPU::Variable<DType>& b)
		{
			auto r = new CPU::Variable<DType>(a._shape);

			// Do the sum
			SHAPE_LOOP(a._shape) {
				r->_values[a._shape.idx(x, y)] = a._values[a._shape.idx(x, y)] + b._values[a._shape.idx(x, y)];
			}

			Tape::current()->push([&a, &b, r]() {
				SHAPE_LOOP(a._shape) {
					a._adjs[a._shape.idx(x, y)] += r->_adjs[a._shape.idx(x, y)];
					b._adjs[a._shape.idx(x, y)] -= r->_adjs[a._shape.idx(x, y)];
				}
			});

			return *r;
		}

		template <typename DType>
		CPU::Variable<DType>& operator*(CPU::Variable<DType>& a, CPU::Variable<DType>& b)
		{
			auto r = new CPU::Variable<DType>(a._shape);

			// Do the sum
			SHAPE_LOOP(a._shape) {
				r->_values[a._shape.idx(x, y)] = a._values[a._shape.idx(x, y)] * b._values[a._shape.idx(x, y)];
			}

			DType* av = a._values;
			DType* bv = b._values;

			Tape::current()->push([av, bv, &a, &b, r]() {
				SHAPE_LOOP(a._shape) {
					a._adjs[a._shape.idx(x, y)] += bv[a._shape.idx(x, y)] * r->_adjs[a._shape.idx(x, y)];
					b._adjs[a._shape.idx(x, y)] += av[a._shape.idx(x, y)] * r->_adjs[a._shape.idx(x, y)];
				}
			});

			return *r;
		}

		template <typename DType>
		CPU::Variable<DType>& operator/(CPU::Variable<DType>& a, CPU::Variable<DType>& b)
		{
			auto r = new CPU::Variable<DType>(a._shape);

			// Do the sum
			SHAPE_LOOP(a._shape) {
				r->_values[a._shape.idx(x, y)] = a._values[a._shape.idx(x, y)] / b._values[a._shape.idx(x, y)];
			}

			DType* av = a._values;
			DType* bv = b._values;

			Tape::current()->push([av, bv, &a, &b, r]() {
				SHAPE_LOOP(a._shape) {
					auto bv2 = bv[a._shape.idx(x, y)] * bv[a._shape.idx(x, y)];

					a._adjs[a._shape.idx(x, y)] += (bv[a._shape.idx(x, y)] * r->_adjs[a._shape.idx(x, y)]) / bv2;
					b._adjs[a._shape.idx(x, y)] -= (av[a._shape.idx(x, y)] * r->_adjs[a._shape.idx(x, y)]) / bv2;
				}
			});

			return *r;
		}

		template <typename DType>
		CPU::Variable<DType>& sqrt(CPU::Variable<DType>& a)
		{
			auto r = new CPU::Variable<DType>(a._shape);

			// Do the sum
			SHAPE_LOOP(a._shape) {
				r->_values[a._shape.idx(x, y)] = std::sqrt(a._values[a._shape.idx(x, y)]);
			}

			DType* rv = r->_values;
			DType* adjs = r->_adjs;

			Tape::current()->push([rv, &a, adjs]() {
				SHAPE_LOOP(a._shape) {
					a._adj[a._shape.idx(x, y)] += adjs[a._shape.idx(x, y)] / (DType(2.0) * rv[a._shape.idx(x, y)]);
				}
			});

			return *r;
		}

		template <typename DType, typename D>
		CPU::Variable<DType>& pow(CPU::Variable<DType>& a, D expo)
		{
			auto r = new CPU::Variable<DType>(a._shape);

			// Do the sum
			SHAPE_LOOP(a._shape) {
				r->_values[a._shape.idx(x, y)] = std::pow(a._values[a._shape.idx(x, y)], expo);
			}

			DType* rv = r->_values;
			DType* adjs = r->_adjs;
			DType* av = a._values;

			Tape::current()->push([rv, &a, av, adjs, expo]() {
				SHAPE_LOOP(a._shape) {
					a._adjs[a._shape.idx(x, y)] += adjs[a._shape.idx(x, y)] * expo * std::pow(av[a._shape.idx(x, y)], DType(expo - 1));
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
			return *(new Bare::CPU::Variable<float>(shape, value));
		}
	}
}

namespace F64
{
	namespace CPU
	{
		inline Bare::CPU::Variable<double>& Variable(Shape shape, double value)
		{
			return *(new Bare::CPU::Variable<double>(shape, value));
		}
	}
}
