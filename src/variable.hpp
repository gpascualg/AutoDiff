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

#define SHAPE_LOOP(shape) for (int y = 0; y < (shape).m; ++y) for (int x = 0; x < (shape).n; ++x)
#define CIDX(shape) shape[{x, y}]

namespace Bare
{
	template <typename DType>
	class Variable
	{
	public:
		explicit Variable(Shape shape):
			_shape(shape)
		{
			_values = new DType[shape.prod()];
			_adjs = new DType[shape.prod()];
		}

		Variable(Shape shape, DType value, DType adj):
			Variable(shape)
		{
			SHAPE_LOOP(shape) {
				values(x, y) = value;
				adjoints(x, y) = adj;
			}
		}

		Variable(Variable& var, DType adj):
			Variable(var.shape())
		{
			SHAPE_LOOP(var.shape()) {
				values(x, y) = var.values(x, y);
				adjoints(x, y) = adj;
			}
		}

		// Slice/Index operator
		Variable(Variable& var, int x0, int y0, int dx, int dy):
			_shape({dx, dy})
		{
			int start = var.shape()[{x0, y0}];
			_values = var._values + start;
			_adjs = var._adjs + start;
			_deleteOnDestructor = false;
		}

		virtual ~Variable()
		{
			if (_deleteOnDestructor)
			{
				delete[] this->_values;
				delete[] this->_adjs;
			}
		}

		inline DType* values() { return _values; }
		inline DType& values(int x, int y) { return _values[_shape[{x, y}]]; }

		inline DType* adjoints() { return _adjs; }
		inline DType& adjoints(int x, int y) { return _adjs[_shape[{x, y}]]; }

		virtual inline void flag()
		{
			SHAPE_LOOP(shape())
			{
				adjoints()[shape()[{x, y}]] = 1;
			}
		}


        inline Shape& shape() { return _shape; }

	protected:
		Shape _shape;

		bool _deleteOnDestructor = true;
		DType* _values = nullptr;
		DType* _adjs = nullptr;
	};
}


#define VARIABLE_INTERFACE(ns,name) \
	template <typename T> friend ns::name<T>& operator+(ns::name<T>& a, ns::name<T>& b);\
	template <typename T> friend ns::name<T>& operator-(ns::name<T>& a, ns::name<T>& b);\
	template <typename T> friend ns::name<T>& operator*(ns::name<T>& a, ns::name<T>& b);\
	template <typename T> friend ns::name<T>& operator/(ns::name<T>& a, ns::name<T>& b);\
	template <typename T> friend ns::name<T>& transpose(ns::name<T>& a);\
	template <typename T> friend ns::name<T>& sqrt(ns::name<T>& a);\
	template <typename T, typename D> friend ns::name<T>& pow(ns::name<T>& a, D expo);\
	template <typename T> friend ns::name<T>& mul(ns::name<T>& a, ns::name<T>& b);\
	template <typename T> friend ns::name<T>& slice(ns::name<T>& a, int x0, int y0, int dx, int dy);\
public:\
	explicit name(Shape shape): \
		Bare::Variable<DType>(shape) \
	{}\
	name(Shape shape, DType value, DType adj=0): \
		Bare::Variable<DType>(shape, value, adj)\
	{}\
	name(Variable& var, DType adj=0):\
		Bare::Variable<DType>(var, adj)\
	{}\
	name(Variable& var, int x0, int y0, int dx, int dy):\
		Bare::Variable<DType>(var, x0, y0, dx, dy)\
	{}
