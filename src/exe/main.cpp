#include <stdio.h>
#include <vector>
#include <functional>

#include <variable.hpp>
#include <cpu.hpp>
#include <blas.hpp>
#include <tape.hpp>

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

	constexpr int dim1 = 5;
	constexpr int dim2 = 5;

	auto x = Variable({dim1, dim2}, 1);
	auto y = Variable({dim1, dim2}, 2);
	auto z = Variable({dim1, dim2}, 2);

	x->values()[3] = 5;

	auto r = x + y + z;

	auto r2 = slice(r, 2, 2, 2, 2);

	printf("r:\n");
	matprint(r->shape(), r->values());

	//getchar();

	r->flag();

	Tape::current()->execute();

	printf("X:\n");
	matprint(x->shape(), x->adjoints());

	printf("Y:\n");
	matprint(y->shape(), y->adjoints());

	printf("Z:\n");
	matprint(z->shape(), z->adjoints());

	// Again!
	r = x + y + z;

	printf("r:\n");
	matprint(r->shape(), r->values());

	//getchar();

	r->flag();

	Tape::current()->execute();

	printf("X:\n");
	matprint(x->shape(), x->adjoints());

	printf("Y:\n");
	matprint(y->shape(), y->adjoints());

	printf("Z:\n");
	matprint(z->shape(), z->adjoints());

	delete Tape::current();

	return 0;
}
