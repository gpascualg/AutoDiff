#include <stdio.h>
#include <vector>
#include <functional>
#include <memory>

#include <variable.hpp>
#include <cpu.hpp>
#include <blas.hpp>
#include <tape.hpp>
#include <optimizer.hpp>


using namespace F32::BLAS;

#if !defined(NDEBUG)
	template <typename DType>
	inline void matprint(Shape shape, void* var)
	{
		DType* dvar = (DType*)var;
		printf("\t%05.2f\n", dvar[0]);
	}
#else
	template <typename DType>
	void matprint(Shape shape, DType* var)
	{
		DType* dvar = (DType*)var;

		for (int y = 0; y < shape.m; ++y)
		{
			printf("\t");
			for (int x = 0; x < shape.n; ++x)
			{
				printf("%06.2f ", dvar[shape[{x, y}]]);
			}
			printf("\n");
		}
		printf("\n");
	}
#endif

int main()
{
	auto tape = std::unique_ptr<Tape>(Tape::use(nullptr));

	constexpr int dim1 = 500;
	constexpr int dim2 = 200;

	auto x = Variable({dim1, dim2}, 1);
	auto y = Variable({dim2, dim1}, 2);
	auto z = Variable({dim1, dim2}, 2);

	x->values(0, 3) = 5;

	auto r = mul(x + z, y) / std::make_shared<Bare::Constant<float>>(100.0);
	//auto r = mul(x + z, y) / std::make_shared<Bare::Constant<float>>(100.0);

	matprint<float>(r->shape(), r->values());

	//Optimizer<float> opt(r, 0.1);

	Tape::current()->execute({ r });

	printf("X:\n");
	matprint<float>(x->shape(), x->adjoints());

	printf("Y:\n");
	matprint<float>(y->shape(), y->adjoints());

	printf("Z:\n");
	matprint<float>(z->shape(), z->adjoints());

	return 0;
}
