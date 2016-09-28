#include <stdio.h>
#include <vector>
#include <functional>

#include "variable.hpp"
#include "cpu.hpp"
#include "blas.hpp"
#include "tape.hpp"

int main()
{
	Tape::use(nullptr);

	using namespace F32::BLAS;

	constexpr int dim1 = 5000;
	constexpr int dim2 = 5000;

	auto x = Variable({dim1, dim2}, 1);
	auto y = Variable({dim1, dim2}, 2);
	auto z = Variable({dim1, dim2}, 2);

	auto& r = (x * y + z) * y;

	printf("%.16f\n", r()[0]);
	//getchar();

	r.flag();

	Tape::current()->execute();

	printf("%.16f\n", x.adj()[0]);
	printf("%.16f\n", y.adj()[0]);
	printf("%.16f\n", z.adj()[0]);

	return 0;
}
