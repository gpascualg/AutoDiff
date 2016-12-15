#include "tape.hpp"
#include "memory_pool.hpp"

#include <algorithm>


Tape* Tape::_last = nullptr;
std::vector<Tape*> Tape::_tapes{};


template <typename DType>
void SpecializedTapeVariable<DType>::update(std::shared_ptr<SpecializedTapeVariable<DType>> var)
{
	Tape::current()->pool()->deallocate<DType>(this->_values, this->shape().prod());
	this->_values = var->_values;
	var->_values = nullptr;
}

Tape::Tape()
{
	_pool = new Pool(true, 1);
}

Tape::~Tape()
{
	// Clean up tape first
	clear();
	close();

	// Clean up pool
	delete _pool;
	_pool = nullptr;

	// Final closeup
	if (_last == this)
	{
		_last = nullptr;
	}
}

void Tape::add(std::shared_ptr<TapeVariable> var)
{
	// TODO(gpascualg): Tape::remove to actually free the variable and the adjoints
	_variables.emplace_back(var);
}

void Tape::execute()
{
	while (!_tape.empty())
	{
		// Call tape.back()
		_tape.back()();

		// Pop function
		_tape.pop_back();
	}
}

void Tape::execute(std::vector<std::shared_ptr<TapeVariable>> targets)
{
	for (auto target : targets)
	{
		target->reset(1);
	}

	for (auto var : _variables)
	{
		auto target = std::find(targets.begin(), targets.end(), var);
		if (target == targets.end())
		{
			var->reset(0);
		}
	}

	execute();
}

void Tape::clear()
{
	while (!_tape.empty())
	{
		_tape.pop_back();
	}

	_variables.clear();
	_pool->release();
}

void Tape::close()
{
	if (_valid)
	{
		_valid = false;
		_tapes.erase(std::find(_tapes.cbegin(), _tapes.cend(), this));
	}
}


template <typename DType>
DType* Tape::getOrCreateAdjoint(const Shape& shape, DType* mem)
{
	auto it = _adjoints.find((intptr_t)mem);
	if (it != _adjoints.end())
	{
		return (DType*)it->second;
	}

	DType* adj = pool()->allocate<DType>(shape.prod());
	_adjoints.emplace((intptr_t)mem, (intptr_t)adj);
	return adj;
}

// PUSH METHODS
template <typename DType>
void Tape::add(DType* result, const Shape& shape, DType* a, DType* b)
{
	DType* adj_a = getOrCreateAdjoint(shape, a);
	DType* adj_b = getOrCreateAdjoint(shape, b);
	DType* adj_r = getOrCreateAdjoint(shape, result);

	Tape::current()->push([adj_r, shape, adj_a, adj_b]() {
		SHAPE_LOOP(shape) {
			// TODO(gpascualg): This should itself be decoupled using Ops::add
			adj_a[CIDX(shape)] += adj_r[CIDX(shape)];
			adj_b[CIDX(shape)] += adj_r[CIDX(shape)];
		}
	});
}

template <typename DType>
void Tape::sub(DType* result, const Shape& shape, DType* a, DType* b)
{
	DType* adj_a = getOrCreateAdjoint(shape, a);
	DType* adj_b = getOrCreateAdjoint(shape, b);
	DType* adj_r = getOrCreateAdjoint(shape, result);

	Tape::current()->push([adj_r, shape, adj_a, adj_b]() {
		SHAPE_LOOP(shape) {
			adj_a[CIDX(shape)] += adj_r[CIDX(shape)];
			adj_b[CIDX(shape)] -= adj_r[CIDX(shape)];
		}
	});
}

template <typename DType>
void Tape::elementwise_mul(DType* result, const Shape& shape_a, DType* a, const Shape& shape_b, DType* b)
{
	DType* adj_a = getOrCreateAdjoint(shape_a, a);
	DType* adj_b = getOrCreateAdjoint(shape_b, b);
	DType* adj_r = getOrCreateAdjoint(shape_a, result);

	Tape::current()->push([adj_r, shape_a, shape_b, a, b, adj_a, adj_b]() {
		SHAPE_LOOP(shape_a) {
			adj_a[CIDX(shape_a)] += b[CIDX(shape_b)] * adj_r[CIDX(shape_a)];
			adj_b[CIDX(shape_a)] += a[CIDX(shape_b)] * adj_r[CIDX(shape_a)];
		}
	});
}

template <typename DType>
void Tape::elementwise_div(DType* result, const Shape& shape_a, DType* a, const Shape& shape_b, DType* b)
{
	DType* adj_a = getOrCreateAdjoint(shape_a, a);
	DType* adj_b = getOrCreateAdjoint(shape_b, b);
	DType* adj_r = getOrCreateAdjoint(shape_a, result);

	Tape::current()->push([adj_r, shape_a, shape_b, a, b, adj_a, adj_b]() {
		SHAPE_LOOP(shape_a) {
			adj_a[CIDX(shape_a)] += b[CIDX(shape_b)] * adj_r[CIDX(shape_a)];
			adj_b[CIDX(shape_a)] -= a[CIDX(shape_b)] * adj_r[CIDX(shape_a)];
		}
	});
}

template <typename DType>
void Tape::sum(DType* result, const Shape& shape, DType* a)
{
}

template <typename DType>
void Tape::sqrt(DType* result, const Shape& shape, DType* a)
{
}

template <typename DType>
void Tape::pow(DType* result, const Shape& shape, DType* a, float expo)
{
}

template <typename DType>
void Tape::mul(DType* result, const Shape& shape_a, DType* a, const Shape& shape_b, DType* b)
{
}

template <typename DType>
void Tape::transpose(DType* result, const Shape& shape, DType* a)
{
}



// SPECIALIZE
template class SpecializedTapeVariable<float>;
template class SpecializedTapeVariable<double>;

template void Tape::add(float* result, const Shape& shape, float* a, float* b);
template void Tape::add(double* result, const Shape& shape, double* a, double* b);

template void Tape::sub(float* result, const Shape& shape, float* a, float* b);
template void Tape::sub(double* result, const Shape& shape, double* a, double* b);

template void Tape::elementwise_mul(float* result, const Shape& shape_a, float* a, const Shape& shape_b, float* b);
template void Tape::elementwise_mul(double* result, const Shape& shape_a, double* a, const Shape& shape_b, double* b);

template void Tape::elementwise_div(float* result, const Shape& shape_a, float* a, const Shape& shape_b, float* b);
template void Tape::elementwise_div(double* result, const Shape& shape_a, double* a, const Shape& shape_b, double* b);

template void Tape::sum(float* result, const Shape& shape, float* a);
template void Tape::sum(double* result, const Shape& shape, double* a);

template void Tape::sqrt(float* result, const Shape& shape, float* a);
template void Tape::sqrt(double* result, const Shape& shape, double* a);

template void Tape::pow(float* result, const Shape& shape, float* a, float expo);
template void Tape::pow(double* result, const Shape& shape, double* a, float expo);

template void Tape::mul(float* result, const Shape& shape_a, float* a, const Shape& shape_b, float* b);
template void Tape::mul(double* result, const Shape& shape_a, double* a, const Shape& shape_b, double* b);

template void Tape::transpose(float* result, const Shape& shape, float* a);
template void Tape::transpose(double* result, const Shape& shape, double* a);
