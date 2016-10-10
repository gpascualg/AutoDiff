#include "tape.hpp"

Tape* Tape::_last = nullptr;
std::vector<Tape*> Tape::_tapes{};

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

void Tape::clear()
{
	while (!_tape.empty())
	{
		_tape.pop_back();
	}
}
