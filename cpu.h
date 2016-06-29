#pragma once

#include "variable.h"
#include "tape.h"

#include <functional>
#include <vector>
#include <functional>

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

		public:
			Variable(DType value, DType adj = 0) :
				_value(value),
				_adj(adj)
			{}

			virtual ~Variable()
			{}

			DType operator()() override { return _value; }
			void flag() override { _adj = 1; }

		protected:
			DType _value;
			DType _adj;
		};


		template <typename DType>
		CPU::Variable<DType>& operator+(CPU::Variable<DType>& a, CPU::Variable<DType>& b)
		{
			auto r = new CPU::Variable<DType>(a._value + b._value, 0);

			Tape::current()->push([&a, &b, r]() {
				a._adj += r->_adj;
				b._adj += r->_adj;
			});

			return *r;
		}

		template <typename DType>
		CPU::Variable<DType>& operator-(CPU::Variable<DType>& a, CPU::Variable<DType>& b)
		{
			auto r = new CPU::Variable<DType>(a._value - b._value, 0);

			Tape::current()->push([&a, &b, r]() {
				a._adj += r->_adj;
				b._adj -= r->_adj;
			});

			return *r;
		}

		template <typename DType>
		CPU::Variable<DType>& operator*(CPU::Variable<DType>& a, CPU::Variable<DType>& b)
		{
			auto r = new CPU::Variable<DType>(a._value * b._value, 0);
			float av = a._value;
			float bv = b._value;

			Tape::current()->push([av, bv, &a, &b, r]() {
				a._adj += bv * r->_adj;
				b._adj += av * r->_adj;
			});

			return *r;
		}

		template <typename DType>
		CPU::Variable<DType>& operator/(CPU::Variable<DType>& a, CPU::Variable<DType>& b)
		{
			auto r = new CPU::Variable<DType>(a._value / b._value, 0);
			float av = a._value;
			float bv = b._value;

			Tape::current()->push([av, bv, &a, &b, r]() {
				a._adj += (bv * r->_adj) / (bv * bv);
				b._adj -= (av * r->_adj) / (bv * bv);
			});

			return *r;
		}

		template <typename DType>
		CPU::Variable<DType>& sqrt(CPU::Variable<DType>& a)
		{
			auto r = new CPU::Variable<DType>(std::sqrt(a._value), 0);
			float rv = r->_value;

			Tape::current()->push([rv, &a, r]() {
				a._adj += r->_adj / (DType(2.0) * rv);
			});

			return *r;
		}
	}
}

namespace F32
{
	namespace CPU
	{
		inline Bare::CPU::Variable<float>& Variable(float value)
		{
			return *(new Bare::CPU::Variable<float>(value));
		}
	}
}

namespace F64
{
	namespace CPU
	{
		inline Bare::CPU::Variable<double>& Variable(double value)
		{
			return *(new Bare::CPU::Variable<double>(value));
		}
	}
}

