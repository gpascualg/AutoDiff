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

template <typename T> class Ops;

template <class T, typename... Args>
std::shared_ptr<T> make_variable(Args&&... args)
{
	auto shared = std::make_shared<T>(std::forward<Args>(args)...);
	Tape::current()->add(shared);
	return shared;
}


// Clang
template <typename T> class Optimizer;


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
			// TODO(gpascualg): Constants should also get a pointer to Ops
			SpecializedTapeVariable<DType>(nullptr, shape)
		{}

		Constant(Shape shape, DType* values) :
			SpecializedTapeVariable<DType>(nullptr, shape)
		{
			this->_values = values;
		}

		Constant(DType value):
			SpecializedTapeVariable<DType>(nullptr, Shape {1, 1})
		{
			this->_values = new DType;
			*this->_values = value;
		}
	};

	template <typename DType>
	class Variable : public SpecializedTapeVariable<DType>
	{
		template <typename T> friend class ::Optimizer;

	public:
		Variable(Ops<DType>* ops, Shape shape):
			SpecializedTapeVariable<DType>(ops, shape)
		{
			this->_values = (DType*)Tape::current()->pool()->allocate<DType>(shape.prod());
		}

		Variable(Ops<DType>* ops, Shape shape, DType* ptr):
			SpecializedTapeVariable<DType>(ops, shape)
		{
			this->_values = ptr;
		}

		Variable(Ops<DType>* ops, int m, int n):
			Variable(ops, Shape {m, n})
		{}

		// TODO(gpascualg): CHECK ALL CONSTRUCTORS ABOVE; SOME DEPEND ON ADJOINTS AND SHOULD NOT!

		Variable(Ops<DType>* ops, Shape shape, DType value, DType adj = 0):
			Variable(ops, shape)
		{
			if (value != 0 || adj != 0)
			{
				SHAPE_LOOP(shape) {
					this->values(x, y) = value;
				}
			}
		}

		Variable(std::shared_ptr<SpecializedTapeVariable<DType>> var, DType adj = 0):
			Variable(var->ops(), var->shape())
		{
			SHAPE_LOOP(var->shape()) {
				this->values(x, y) = var->values(x, y);
			}
		}

		// Slice/Index operator
		Variable(std::shared_ptr<SpecializedTapeVariable<DType>> var, int x0, int y0, int dx, int dy):
			SpecializedTapeVariable<DType>(var->ops(), Shape{dx, dy})
		{
			int start = var->shape()[{x0, y0}];
			this->_values = var->_values + start;
			_deleteOnDestructor = false;
		}

		Variable(std::shared_ptr<Variable> var, DType adj = 0):
			Variable(var->ops(), var->shape())
		{
			SHAPE_LOOP(var->shape()) {
				this->values(x, y) = var->values(x, y);
			}
		}

		// Slice/Index operator
		Variable(std::shared_ptr<Variable> var, int x0, int y0, int dx, int dy):
			SpecializedTapeVariable<DType>(var->ops(), Shape{dx, dy})
		{
			int start = var->shape()[{x0, y0}];
			this->_values = var->_values + start;
			// TODO(gpascualg): ADJOINT SLICING
			_deleteOnDestructor = false;
		}

		virtual ~Variable()
		{
			if (_deleteOnDestructor && Tape::current())
			{
				if (this->_values)
				{
					// TODO(gpascualg): Remove Variable from Tape
					Tape::current()->pool()->deallocate<DType>(this->_values, this->shape().prod());
				}

				// TODO(gpascualg): Adjoint freeing
			}
		}

		void reset(float to) override
		{
			DType* adj = Tape::current()->getOrCreateAdjoint(this->shape(), this->values());

			SELF_SHAPE_LOOP()
			{
				adj[CIDX(this->shape())] = DType(to);
			}
		}

	protected:
		bool _deleteOnDestructor = true;
	};
}
