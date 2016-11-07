#pragma once

#include "config.h"
#include "tape.hpp" // TODO(gpascualg): Move TapeVariable to aother file
#include "memory_pool.hpp"

#include <memory>
#include <type_traits>


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
	template <typename T> friend std::shared_ptr<Variable<T>> operator*(std::shared_ptr<Variable<T>> a, std::shared_ptr<Bare::Constant<T>> b); \
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
	DEFINE_FRIEND_CONST(operator*, elementwise_mul, Variable); \
	DEFINE_FRIEND_2(operator/, elementwise_div, Variable); \
	DEFINE_FRIEND_CONST(operator/, elementwise_div, Variable); \
	DEFINE_FRIEND_2(mul, mul, Variable); \
	DEFINE_FRIEND_2_(pow, pow, Variable, float, ); \
	DEFINE_FRIEND_1(transpose, transpose, Variable); \
	DEFINE_FRIEND_1(sqrt, sqrt, Variable); \
	DEFINE_FRIEND_1(sum, sum, Variable);


namespace Bare
{
	template <typename T> using SharedTapeVariable = std::shared_ptr<SpecializedTapeVariable<T>>;
	template <typename T> using SharedVariable = std::shared_ptr<Bare::Variable<T>>;
	template <typename T> using SharedConstant = std::shared_ptr<Bare::Constant<T>>;

	template <typename DType>
	class Constant: public SpecializedTapeVariable<DType>
	{
	public:
		Constant(const Shape& shape):
			SpecializedTapeVariable<DType>(shape)
		{}

		Constant(Shape shape, DType* values) :
			SpecializedTapeVariable<DType>(shape)
		{
			this->_values = values;
		}

		Constant(DType value):
			SpecializedTapeVariable<DType>(Shape {1, 1})
		{
			this->_values = new DType;
			*this->_values = value;
		}
	};

	template <typename DType>
	class Variable : public SpecializedTapeVariable<DType>
	{
		template <typename T> friend class Optimizer;

	public:
		explicit Variable(Shape shape):
			SpecializedTapeVariable<DType>(shape)
		{
			this->_values = (DType*)Tape::current()->pool()->allocate<DType>(shape.prod());
			this->_adjs = (DType*)Tape::current()->pool()->allocate<DType>(shape.prod());
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
					this->values(x, y) = value;
					this->adjoints(x, y) = adj;
				}
			}
		}

		Variable(std::shared_ptr<SpecializedTapeVariable<DType>> var, DType adj = 0):
			Variable(var->shape())
		{
			SHAPE_LOOP(var->shape()) {
				this->values(x, y) = var->values(x, y);
				this->adjoints(x, y) = adj;
			}
		}

		// Slice/Index operator
		Variable(std::shared_ptr<SpecializedTapeVariable<DType>> var, int x0, int y0, int dx, int dy):
			SpecializedTapeVariable<DType>(Shape{dx, dy})
		{
			int start = var->shape()[{x0, y0}];
			this->_values = var->_values + start;
			this->_adjs = var->_adjs + start;
			_deleteOnDestructor = false;
		}

		Variable(std::shared_ptr<Variable> var, DType adj = 0):
			Variable(var->shape())
		{
			SHAPE_LOOP(var->shape()) {
				this->values(x, y) = var->values(x, y);
				this->adjoints(x, y) = adj;
			}
		}

		// Slice/Index operator
		Variable(std::shared_ptr<Variable> var, int x0, int y0, int dx, int dy):
			SpecializedTapeVariable<DType>(Shape{dx, dy})
		{
			int start = var->shape()[{x0, y0}];
			this->_values = var->_values + start;
			this->_adjs = var->_adjs + start;
			_deleteOnDestructor = false;
		}

		virtual ~Variable()
		{
			if (_deleteOnDestructor && Tape::current())
			{
				if (this->_values)
				{
					Tape::current()->pool()->deallocate<DType>(this->_values, this->shape().prod());
				}

				if (this->_adjs)
				{
					Tape::current()->pool()->deallocate<DType>(this->_adjs, this->shape().prod());
				}
			}
		}

		virtual inline void flag()
		{
			SELF_SHAPE_LOOP()
			{
				this->adjoints(x, y) = 1;
			}
		}

		void reset(float to) override
		{
			SELF_SHAPE_LOOP()
			{
				this->adjoints(x, y) = DType(to);
			}
		}


	protected:
		// Adjoints calculation
		virtual SharedVariable<DType> add(SharedVariable<DType> a, SharedVariable<DType> b) = 0;
		virtual SharedVariable<DType> sub(SharedVariable<DType> a, SharedVariable<DType> b) = 0;
		virtual SharedVariable<DType> elementwise_mul(SharedVariable<DType> a, SharedVariable<DType> b) = 0;
		virtual SharedVariable<DType> elementwise_mul(SharedVariable<DType> a, SharedConstant<DType> b) = 0;
		virtual SharedVariable<DType> elementwise_div(SharedVariable<DType> a, SharedVariable<DType> b) = 0;
		virtual SharedVariable<DType> elementwise_div(SharedVariable<DType> a, SharedConstant<DType> b) = 0;
		virtual SharedVariable<DType> mul(SharedVariable<DType> a, SharedVariable<DType> b) = 0;

		virtual SharedVariable<DType> sum(SharedVariable<DType> a) = 0;
		virtual SharedVariable<DType> pow(SharedVariable<DType> a, float expo) = 0;
		virtual SharedVariable<DType> sqrt(SharedVariable<DType> a) = 0;
		virtual SharedVariable<DType> transpose(SharedVariable<DType> a) = 0;

		// Vanilla operations
		virtual SharedTapeVariable<DType> _add(SharedTapeVariable<DType> a, SharedTapeVariable<DType> b) = 0;
		virtual SharedTapeVariable<DType> _sub(SharedTapeVariable<DType> a, SharedTapeVariable<DType> b) = 0;
		virtual SharedTapeVariable<DType> _elementwise_mul(SharedTapeVariable<DType> a, SharedTapeVariable<DType> b) = 0;
		virtual SharedTapeVariable<DType> _elementwise_div(SharedTapeVariable<DType> a, SharedTapeVariable<DType> b) = 0;
		virtual SharedTapeVariable<DType> _mul(SharedTapeVariable<DType> a, SharedTapeVariable<DType> b) = 0;

		virtual SharedTapeVariable<DType> _sum(SharedTapeVariable<DType> a) = 0;
		virtual SharedTapeVariable<DType> _pow(SharedTapeVariable<DType> a, float expo) = 0;
		virtual SharedTapeVariable<DType> _sqrt(SharedTapeVariable<DType> a) = 0;
		virtual SharedTapeVariable<DType> _transpose(SharedTapeVariable<DType> a) = 0;


	protected:
		bool _deleteOnDestructor = true;
	};
}
