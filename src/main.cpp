#include <stdio.h>
#include <vector>
#include <functional>

#include "variable.hpp"
#include "cpu.hpp"
#include "blas.hpp"
#include "tape.hpp"

using namespace F32::BLAS;

#if defined(NDEBUG)
	#define matprint(shape, var) printf("\t%05.2f\n", var[0]);
#else
	template <typename DType>
	void matprint(Shape shape, DType* var)
	{
		for (int y = 0; y < shape.m; ++y)
		{
			printf("\t");
			for (int x = 0; x < shape.n; ++x)
			{
				printf("%06.2f ", var[shape[{x, y}]]);
			}
			printf("\n");
		}
		printf("\n");
	}
#endif

int main()
{
	Tape::use(nullptr);

	constexpr int dim1 = 3;
	constexpr int dim2 = 7;

	auto& x = Variable({dim1, dim2}, 1);
	auto& y = Variable({dim1, dim2}, 2);
	auto& z = Variable({dim1, dim2}, 2);

	x()[3] = 5;

	auto& r = x * y + z + pow(x, 4.0);

	printf("r:\n");
	matprint(r.shape(), r());

	//getchar();

	r.flag();

	Tape::current()->execute();

	printf("X:\n");
	matprint(x.shape(), x.adj());

	printf("Y:\n");
	matprint(y.shape(), y.adj());

	printf("Z:\n");
	matprint(z.shape(), z.adj());

	getchar();

	return 0;
}
