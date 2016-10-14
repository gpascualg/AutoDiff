#include "tape.hpp"

#include <algorithm>

Tape* Tape::_last = nullptr;
std::vector<Tape*> Tape::_tapes{};

Tape::~Tape()
{
	clear();
	close();
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
	for (auto var : _variables)
	{
		auto target = std::find(targets.begin(), targets.end(), var);
		if (target != targets.end())
		{
			(*target)->reset(1);
			targets.erase(target);
		}
		else
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
}

void Tape::close()
{
	if (_valid)
	{
		_valid = false;
		_tapes.erase(std::find(_tapes.cbegin(), _tapes.cend(), this));
	}
}
