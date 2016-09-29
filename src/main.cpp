#include <stdio.h>
#include <vector>
#include <functional>

#include "variable.hpp"
#include "cpu.hpp"
#include "blas.hpp"
#include "tape.hpp"

using namespace F32::BLAS;

template <typename DType>
void matprint(Bare::Variable<DType>& var)
{
	for (int y = 0; y < var.shape().m; ++y)
	{
		for (int x = 0; x < var.shape().n; ++x)
		{
			printf("%05.2f ", var()[var.shape()[{x, y}]]);
		}
		printf("\n");
	}
	printf("\n");
}

int main()
{
	Tape::use(nullptr);

	constexpr int dim1 = 3;
	constexpr int dim2 = 4;

	auto& x = Variable({dim1, dim2}, 1);
	auto& y = Variable({dim1, dim2}, 2);
	auto& z = Variable({dim1, dim2}, 2);

	x()[3] = 5;

	auto& r = x + y;

	matprint(r);

	//getchar();

	r.flag();

	Tape::current()->execute();

	printf("%.16f\n", x.adj()[0]);
	printf("%.16f\n", y.adj()[0]);
	printf("%.16f\n", z.adj()[0]);

	return 0;
}
