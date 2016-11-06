#include "tape.hpp"
#include "memory_pool.hpp"

#include <algorithm>


Tape* Tape::_last = nullptr;
std::vector<Tape*> Tape::_tapes{};

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
