#include "tape.h"

Tape* Tape::_last = nullptr;
std::vector<Tape*> Tape::_tapes{};

void Tape::execute()
{
	while (!_tape.empty())
	{
		const std::function<void()>& calc_adj = _tape.back();
		calc_adj();
		_tape.pop_back();
	}
}
