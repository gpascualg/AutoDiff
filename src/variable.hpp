#pragma once

struct Shape
{
	explicit constexpr Shape() : Shape(0, 0) {}

	constexpr Shape(int m, int n):
		m(m), n(n)
	{}

	inline constexpr int prod() const { return m*n; }
	inline constexpr Shape T() const { return { n, m }; }
	inline constexpr int idx(int x, int y) const { return y * n + x; }

	inline constexpr int operator[](Shape&& shape) { return idx(shape.m, shape.n); }

	int m;
	int n;
};

#define SHAPE_LOOP(shape) for (int y = 0; y < shape.m; ++y) for (int x = 0; x < shape.n; ++x)

namespace Bare
{
	template <typename DType>
	class Variable
	{
	public:
		explicit Variable(Shape shape):
			_shape(shape)
		{}

		virtual ~Variable()
		{}

		virtual DType* operator()() = 0;
		virtual DType* adj() = 0;
		virtual void flag() = 0;


        Shape& shape() { return _shape; }

	protected:
		Shape _shape;
	};
}
