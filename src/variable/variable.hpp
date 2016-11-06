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
	template <typename T> class Constant;
}

template <class T, typename... Args>
std::shared_ptr<T> make_variable(Args&&... args)
{
	auto shared = std::make_shared<T>(std::forward<Args>(args)...);
	Tape::current()->add(shared);
	return shared;
}


#define ADD_FRIENDS(Variable) \
	template <typename T> friend std::shared_ptr<Variable<T>> operator+(std::shared_ptr<Variable<T>> a, std::shared_ptr<Variable<T>> b); \
	template <typename T> friend std::shared_ptr<Variable<T>> operator-(std::shared_ptr<Variable<T>> a, std::shared_ptr<Variable<T>> b); \
	template <typename T> friend std::shared_ptr<Variable<T>> operator*(std::shared_ptr<Variable<T>> a, std::shared_ptr<Variable<T>> b); \
	template <typename T> friend std::shared_ptr<Variable<T>> operator/(std::shared_ptr<Variable<T>> a, std::shared_ptr<Variable<T>> b); \
	template <typename T> friend std::shared_ptr<Variable<T>> operator/(std::shared_ptr<Variable<T>> a, std::shared_ptr<Bare::Constant<T>> b); \
	template <typename T> friend std::shared_ptr<Variable<T>> mul(std::shared_ptr<Variable<T>> a, std::shared_ptr<Variable<T>> b); \
	template <typename T> friend std::shared_ptr<Variable<T>> pow(std::shared_ptr<Variable<T>> a, float); \
	template <typename T> friend std::shared_ptr<Variable<T>> transpose(std::shared_ptr<Variable<T>> a); \
	template <typename T> friend std::shared_ptr<Variable<T>> sqrt(std::shared_ptr<Variable<T>> a); \
	template <typename T> friend std::shared_ptr<Variable<T>> sum(std::shared_ptr<Variable<T>> a);


#define DEFINE_FRIEND_1(name, member, Variable) \
	template <typename T> std::shared_ptr<Variable<T>> name(std::shared_ptr<Variable<T>> a) \
	{ \
		return std::dynamic_pointer_cast<Variable<T>>(a->member(std::dynamic_pointer_cast<Variable<T>>(a))); \
	}


#define DEFINE_FRIEND_2_(name, member, Variable, BType, BCast) \
	template <typename T> std::shared_ptr<Variable<T>> name(std::shared_ptr<Variable<T>> a, BType b) \
	{ \
		return std::dynamic_pointer_cast<Variable<T>>(a->member(std::dynamic_pointer_cast<Variable<T>>(a), BCast(b))); \
	}

#define DEFINE_FRIEND_CONST(name, member, Variable) \
	DEFINE_FRIEND_2_(name, member, Variable, std::shared_ptr<Constant<T>>, )


#define DEFINE_FRIEND_2(name, member, Variable) \
	DEFINE_FRIEND_2_(name, member, Variable, std::shared_ptr<Variable<T>>, std::dynamic_pointer_cast<Variable<T>>)


#define DECLARE_FRIENDS(Variable) \
	DEFINE_FRIEND_2(operator+, add, Variable); \
	DEFINE_FRIEND_2(operator-, sub, Variable); \
	DEFINE_FRIEND_2(operator*, elementwise_mul, Variable); \
	DEFINE_FRIEND_2(operator/, elementwise_div, Variable); \
	DEFINE_FRIEND_CONST(operator/, elementwise_div, Variable); \
	DEFINE_FRIEND_2(mul, mul, Variable); \
	DEFINE_FRIEND_2_(pow, pow, Variable, float, ); \
	DEFINE_FRIEND_1(transpose, transpose, Variable); \
	DEFINE_FRIEND_1(sqrt, sqrt, Variable); \
	DEFINE_FRIEND_1(sum, sum, Variable);


namespace Bare
{
	template <typename T> using SharedVariable = std::shared_ptr<Bare::Variable<T>>;
	template <typename T> using SharedConstant = std::shared_ptr<Bare::Constant<T>>;

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
		// Adjoints calculation
		virtual SharedVariable<DType> add(SharedVariable<DType> a, SharedVariable<DType> b) = 0;
		virtual SharedVariable<DType> sub(SharedVariable<DType> a, SharedVariable<DType> b) = 0;
		virtual SharedVariable<DType> elementwise_mul(SharedVariable<DType> a, SharedVariable<DType> b) = 0;
		virtual SharedVariable<DType> elementwise_div(SharedVariable<DType> a, SharedVariable<DType> b) = 0;
		virtual SharedVariable<DType> elementwise_div(SharedVariable<DType> a, SharedConstant<DType> b) = 0;
		virtual SharedVariable<DType> mul(SharedVariable<DType> a, SharedVariable<DType> b) = 0;

		virtual SharedVariable<DType> sum(SharedVariable<DType> a) = 0;
		virtual SharedVariable<DType> pow(SharedVariable<DType> a, float expo) = 0;
		virtual SharedVariable<DType> sqrt(SharedVariable<DType> a) = 0;
		virtual SharedVariable<DType> transpose(SharedVariable<DType> a) = 0;

		// Vanilla operations
		virtual SharedVariable<DType> _add(SharedVariable<DType> a, SharedVariable<DType> b) = 0;
		virtual SharedVariable<DType> _sub(SharedVariable<DType> a, SharedVariable<DType> b) = 0;
		virtual SharedVariable<DType> _elementwise_mul(SharedVariable<DType> a, SharedVariable<DType> b) = 0;
		virtual SharedVariable<DType> _elementwise_div(SharedVariable<DType> a, SharedVariable<DType> b) = 0;
		virtual SharedVariable<DType> _elementwise_div(SharedVariable<DType> a, SharedConstant<DType> b) = 0;
		virtual SharedVariable<DType> _mul(SharedVariable<DType> a, SharedVariable<DType> b) = 0;

		virtual SharedVariable<DType> _sum(SharedVariable<DType> a) = 0;
		virtual SharedVariable<DType> _pow(SharedVariable<DType> a, float expo) = 0;
		virtual SharedVariable<DType> _sqrt(SharedVariable<DType> a) = 0;
		virtual SharedVariable<DType> _transpose(SharedVariable<DType> a) = 0;


	protected:
		Shape _shape;

		bool _deleteOnDestructor = true;
		bool _isTrainable = true;
		DType* _values = nullptr;
		DType* _adjs = nullptr;
	};
}

// DEFINE_FRIEND_2(operator+, add, Bare::Variable);
// DEFINE_FRIEND_2(operator-, sub, Bare::Variable);
// DEFINE_FRIEND_2(operator*, mul, Bare::Variable);
// DEFINE_FRIEND_2(operator/, div, Bare::Variable);
// DEFINE_FRIEND_2(mul, mul, Bare::Variable);
