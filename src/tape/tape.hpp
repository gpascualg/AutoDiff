#pragma once

#include <vector>
#include <functional>
#include <memory>


class Pool;
class Tape;

struct Shape
{
	explicit constexpr Shape() : Shape(0, 0) {}

	constexpr Shape(int m, int n):
		m(m), n(n)
	{}

	constexpr Shape(const Shape& shape):
		Shape(shape.m, shape.n)
	{}

	inline constexpr int prod() const { return m*n; }
	inline constexpr Shape T() const { return { n, m }; }
	inline constexpr int idx(int x, int y) const { return y * n + x; }

	inline constexpr int operator[](Shape&& shape) { return idx(shape.m, shape.n); }

	inline constexpr bool isUnitary() const { return m == n && n == 1; }

	int const m;
	int const n;
};

#define SHAPE_LOOP(shape) for (int y = 0; y < (shape).m; ++y) for (int x = 0; x < (shape).n; ++x)
#define SELF_SHAPE_LOOP() SHAPE_LOOP(this->shape())
#define CIDX(shape) shape[{x, y}]


class TapeVariable
{
public:
	TapeVariable():
		_isTrainable()
	{}

	explicit TapeVariable(Shape shape):
	 	_shape(shape)
	{}

	virtual void reset(float to) {};
	virtual inline void* untyped_values() = 0;
	virtual inline void* untyped_adjoints() = 0;
	virtual inline const Shape& shape() const { return _shape; }


	void setTrainable(bool isTrainable) { _isTrainable = isTrainable; }
	inline bool isTrainable() { return _isTrainable; }

protected:
	Shape _shape;
	bool _isTrainable = true;
};

template <typename DType>
class SpecializedTapeVariable: public TapeVariable
{
public:
	using TapeVariable::TapeVariable;

	inline void* untyped_values() override { return (void*)_values; }
	inline DType* values() { return _values; }
	inline DType& values(int x, int y) { return _values[_shape[{x, y}]]; }

	inline void* untyped_adjoints() override { return (void*)_adjs; }
	inline DType* adjoints() { return _adjs; }
	inline DType& adjoints(int x, int y) { return _adjs[_shape[{x, y}]]; }

	void update(std::shared_ptr<SpecializedTapeVariable<DType>> var);

protected:
	DType* _values = nullptr;
	DType* _adjs = nullptr;
};

class Tape
{
public:
	~Tape();

	static Tape* use(Tape* which, void* mem = nullptr)
	{
		if (which && !which->_valid)
		{
			return nullptr;
		}

		if (!which)
		{
			if (!mem)
			{
				mem = malloc(sizeof(Tape));
			}

			which = new (mem) Tape();
			_tapes.push_back(which);
		}
		else if (mem)
		{
			// TODO(gpascualg): UNSAFE!
			*(Tape*)mem = *which;
		}

		_last = which;
		return which;
	}

	static Tape* current()
	{
		return _last;
	}

	template <typename T>
	void push(T&& adj_calc)
	{
		_tape.emplace_back(std::move(adj_calc));
	}

	template <typename T>
	void prepend(T&& adj_calc)
	{
		_tape.emplace(_tape.begin(), std::move(adj_calc));
	}

	void add(std::shared_ptr<TapeVariable> var);

	void execute();
	void execute(std::vector<std::shared_ptr<TapeVariable>> targets);

	inline int size() { return _tape.size(); }
	void clear();
	void close();

	inline Pool* pool()
	{
		return _pool;
	}

	inline std::vector<std::shared_ptr<TapeVariable>>& variables()
	{
		return _variables;
	}

private:
	Tape();

private:
	std::vector<std::function<void()>> _tape;
	std::vector<std::shared_ptr<TapeVariable>> _variables;

private:
	static Tape* _last;
	static std::vector<Tape*> _tapes;

	Pool* _pool;
	bool _valid = true;
};
