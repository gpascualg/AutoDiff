#pragma once

#include "config.h"

#include <memory>

struct Shape
{
	explicit constexpr Shape() : Shape(0, 0) {}

	constexpr Shape(int m, int n):
		m(m), n(n)
	{}

	inline constexpr int prod() const { return m*n; }
	inline constexpr Shape T() const { return { n, m }; }
	inline constexpr int idx(int x, int y) const { return y * n + x; }

	inline constexpr int operator[](Shape&& shape) { return idx(shape.m, shape.n); }

	int const m;
	int const n;
};

#define SHAPE_LOOP(shape) for (int y = 0; y < (shape).m; ++y) for (int x = 0; x < (shape).n; ++x)
#define CIDX(shape) shape[{x, y}]

namespace Bare
{
	template <typename DType>
	class Variable
	{
	public:
		explicit Variable(Shape shape):
			_shape(shape)
		{
			_values = new DType[shape.prod()];
			_adjs = new DType[shape.prod()];
		}

		Variable(int m, int n):
			Variable(Shape {m, n})
		{}

		Variable(Shape shape, DType value, DType adj = 0):
			Variable(shape)
		{
			SHAPE_LOOP(shape) {
				values(x, y) = value;
				adjoints(x, y) = adj;
			}
		}

		Variable(std::shared_ptr<Variable> var, DType adj = 0):
			Variable(var->shape())
		{
			SHAPE_LOOP(var->shape()) {
				values(x, y) = var->values(x, y);
				adjoints(x, y) = adj;
			}
		}

		// Slice/Index operator
		Variable(std::shared_ptr<Variable> var, int x0, int y0, int dx, int dy):
			_shape({dx, dy})
		{
			int start = var->shape()[{x0, y0}];
			_values = var->_values + start;
			_adjs = var->_adjs + start;
			_deleteOnDestructor = false;
		}

		virtual ~Variable()
		{
			if (_deleteOnDestructor)
			{
				delete[] this->_values;
				delete[] this->_adjs;
			}
		}

		inline DType* values() { return _values; }
		inline DType& values(int x, int y) { return _values[_shape[{x, y}]]; }

		inline DType* adjoints() { return _adjs; }
		inline DType& adjoints(int x, int y) { return _adjs[_shape[{x, y}]]; }

		virtual inline void flag()
		{
			SHAPE_LOOP(shape())
			{
				adjoints()[shape()[{x, y}]] = 1;
			}
		}


        inline Shape& shape() { return _shape; }

	protected:
		Shape _shape;

		bool _deleteOnDestructor = true;
		DType* _values = nullptr;
		DType* _adjs = nullptr;
	};
}


#define VARIABLE_INTERFACE(ns,name) \
	template <typename T> friend std::shared_ptr<ns::name<T>> operator+(std::shared_ptr<ns::name<T>> a, std::shared_ptr<ns::name<T>> b);\
	template <typename T> friend std::shared_ptr<ns::name<T>> operator-(std::shared_ptr<ns::name<T>> a, std::shared_ptr<ns::name<T>> b);\
	template <typename T> friend std::shared_ptr<ns::name<T>> operator*(std::shared_ptr<ns::name<T>> a, std::shared_ptr<ns::name<T>> b);\
	template <typename T> friend std::shared_ptr<ns::name<T>> operator/(std::shared_ptr<ns::name<T>> a, std::shared_ptr<ns::name<T>> b);\
	template <typename T> friend std::shared_ptr<ns::name<T>> transpose(std::shared_ptr<ns::name<T>> a);\
	template <typename T> friend std::shared_ptr<ns::name<T>> sqrt(std::shared_ptr<ns::name<T>> a);\
	template <typename T, typename D> friend std::shared_ptr<ns::name<T>> pow(std::shared_ptr<ns::name<T>> a, D expo);\
	template <typename T> friend std::shared_ptr<ns::name<T>> mul(std::shared_ptr<ns::name<T>> a, std::shared_ptr<ns::name<T>> b);\
	template <typename T> friend std::shared_ptr<ns::name<T>> slice(std::shared_ptr<ns::name<T>> a, int x0, int y0, int dx, int dy);\
public:\
	using Bare::Variable<DType>::Variable;
