#pragma once

#include "config.h"
#include "tape.hpp" // TODO(gpascualg): Move TapeVariable to aother file
#include "memory_pool.hpp"

#include <memory>
#include <type_traits>


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
	template <typename T> class Variable;
}

template <class T, typename... Args>
std::shared_ptr<T> make_variable(Args&&... args)
{
	auto shared = std::make_shared<T>(std::forward<Args>(args)...);
	Tape::current()->add(shared);
	return shared;
}


namespace Bare
{
	template <typename T>
	using SharedVariable = std::shared_ptr<Bare::Variable<T>>;

	template <typename DType>
	class Constant
	{
	public:
		Constant(DType value):
			_value(value)
		{}

		inline const DType value() const { return _value; }

	private:
		const DType _value;
	};

	template <typename DType>
	class Variable : public TapeVariable
	{
	public:
		explicit Variable(Shape shape):
			_shape(shape)
		{
			_values = (DType*)Tape::current()->pool()->allocate<DType>(shape.prod());
			_adjs = (DType*)Tape::current()->pool()->allocate<DType>(shape.prod());
		}

		Variable(int m, int n):
			Variable(Shape {m, n})
		{}

		Variable(Shape shape, DType value, DType adj = 0):
			Variable(shape)
		{
			if (value != 0 || adj != 0)
			{
				SHAPE_LOOP(shape) {
					values(x, y) = value;
					adjoints(x, y) = adj;
				}
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
			if (_deleteOnDestructor && Tape::current())
			{
				Tape::current()->pool()->deallocate<DType>(this->_values, _shape.prod());
				Tape::current()->pool()->deallocate<DType>(this->_adjs, _shape.prod());
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

		void reset(float to) override
		{
			SHAPE_LOOP(shape())
			{
				adjoints()[shape()[{x, y}]] = DType(to);
			}
		}

		void setTrainable(bool isTrainable) { _isTrainable = isTrainable; }

        inline Shape& shape() { return _shape; }
		inline bool isTrainable() { return _isTrainable; }

	protected:


	protected:
		Shape _shape;

		bool _deleteOnDestructor = true;
		bool _isTrainable = true;
		DType* _values = nullptr;
		DType* _adjs = nullptr;
	};
}


#define VARIABLE_INTERFACE(ns,name) \
	template <typename T> friend std::shared_ptr<ns::name<T>> operator+(std::shared_ptr<ns::name<T>> a, std::shared_ptr<ns::name<T>> b);\
	template <typename T> friend std::shared_ptr<ns::name<T>> operator-(std::shared_ptr<ns::name<T>> a, std::shared_ptr<ns::name<T>> b);\
	template <typename T> friend std::shared_ptr<ns::name<T>> operator*(std::shared_ptr<ns::name<T>> a, std::shared_ptr<ns::name<T>> b);\
	template <typename T> friend std::shared_ptr<ns::name<T>> operator/(std::shared_ptr<ns::name<T>> a, std::shared_ptr<ns::name<T>> b);\
	template <typename T> friend std::shared_ptr<ns::name<T>> operator/(std::shared_ptr<ns::name<T>> a, std::shared_ptr<ns::name<T>> b);\
	template <typename T> friend std::shared_ptr<ns::name<T>> sum(std::shared_ptr<ns::name<T>> a);\
	template <typename T> friend std::shared_ptr<ns::name<T>> transpose(std::shared_ptr<ns::name<T>> a);\
	template <typename T> friend std::shared_ptr<ns::name<T>> sqrt(std::shared_ptr<ns::name<T>> a);\
	template <typename T, typename D> friend std::shared_ptr<ns::name<T>> pow(std::shared_ptr<ns::name<T>> a, D expo);\
	template <typename T> friend std::shared_ptr<ns::name<T>> mul(std::shared_ptr<ns::name<T>> a, std::shared_ptr<ns::name<T>> b);\
	template <typename T> friend std::shared_ptr<ns::name<T>> slice(std::shared_ptr<ns::name<T>> a, int x0, int y0, int dx, int dy);\
public:\
	using Bare::Variable<DType>::Variable;
