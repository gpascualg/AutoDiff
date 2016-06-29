#include <stdio.h>
#include <vector>
#include <functional>

#include "variable.h"
#include "cpu.h"
#include "tape.h"

int main()
{
	Tape::use(nullptr);

	using namespace F32::CPU;

	auto x = Variable(3);
	auto y = Variable(4);
	auto z = Variable(6);
	
	auto& r = sqrt(x) * y;

	printf("%f\n", r());
	getchar();

	r.flag();

	Tape::current()->execute();

	return 0;
}
