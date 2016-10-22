#pragma once

#include <vector>
#include <functional>
#include <memory>


class Pool;

class TapeVariable
{
public:
	TapeVariable() {}

	virtual void reset(float to) = 0;
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

	void add(std::shared_ptr<TapeVariable> var);

	void execute();
	void execute(std::vector<std::shared_ptr<TapeVariable>> targets);

	void clear();
	void close();

	inline Pool* pool()
	{
		return _pool;
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
