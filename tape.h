#pragma once

#include <vector>
#include <functional>

class Tape
{
public:
	static Tape* use(Tape* which)
	{
		if (!which)
		{
			which = new Tape();
			_tapes.push_back(which);
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
		_tape.emplace_back(adj_calc);
	}

	void execute();

private:
	std::vector<std::function<void()>> _tape;
	
private:
	static Tape* _last;
	static std::vector<Tape*> _tapes;
};