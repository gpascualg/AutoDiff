#pragma once

#include "variable.hpp"
#include "tape.hpp"

#include <functional>
#include <vector>
#include <cmath>

#include <cblas.h>



namespace Bare
{
	namespace BLAS
	{
		template <typename DType>
		class Variable : public Bare::Variable<DType>
		{
			template <typename T> friend BLAS::Variable<T>& operator+(BLAS::Variable<T>& a, BLAS::Variable<T>& b);
			template <typename T> friend BLAS::Variable<T>& operator-(BLAS::Variable<T>& a, BLAS::Variable<T>& b);
			template <typename T> friend BLAS::Variable<T>& operator*(BLAS::Variable<T>& a, BLAS::Variable<T>& b);
			template <typename T> friend BLAS::Variable<T>& operator/(BLAS::Variable<T>& a, BLAS::Variable<T>& b);
			template <typename T> friend BLAS::Variable<T>& sqrt(BLAS::Variable<T>& a);
			template <typename T, typename D> friend BLAS::Variable<T>& pow(BLAS::Variable<T>& a, D expo);

		public:
			Variable(Shape shape, DType value = 0, DType adj = 0) :
				_shape(shape)
			{
				_values = new DType[shape.prod()];
				_adjs = new DType[shape.prod()];

				SHAPE_LOOP(shape) {
					_values[shape.idx(x, y)] = value;
					_adjs[shape.idx(x, y)] = adj;
				}
			}

            Variable(Variable<DType>* other, DType adj = 0, bool copy = true) :
                _shape(other->_shape)
            {
                _values = new DType[other->_shape.prod()];
                _adjs = new DType[other->_shape.prod()];

                if (copy)
                {
    				SHAPE_LOOP(other->_shape) {
    					_values[other->_shape.idx(x, y)] = other->_values[other->_shape.idx(x, y)];
    					_adjs[other->_shape.idx(x, y)] = adj;
    				}
                }
                else
                {
                    SHAPE_LOOP(other->_shape) {
    					_values[other->_shape.idx(x, y)] = 0;
    					_adjs[other->_shape.idx(x, y)] = adj;
    				}
                }
            }

			virtual ~Variable()
			{
				delete[] _values;
				delete[] _adjs;
			}

            Shape& shape() { return _shape; }
			DType* operator()() override { return _values; }
			DType* adj() override { return _adjs; }

			void flag() override
			{
				// Set all adjacent values to 1
				SHAPE_LOOP(_shape) {
					_adjs[_shape.idx(x, y)] = 1;
				}
			}

		protected:
			Shape _shape;
			DType* _values;
			DType* _adjs;
		};


        // BLAS SPECIFIC METHODS
        template <typename DType>
        void cblas_axpy(Shape& shape, DType* a, DType* b, DType scale = 1.0);

        template <> void cblas_axpy(Shape& shape, float* a, float* b, float scale)
        {
            cblas_saxpy(shape.prod(), scale, b, 1, a, 1);
        }
        template <> void cblas_axpy(Shape& shape, double* a, double* b, double scale)
        {
            cblas_daxpy(shape.prod(), scale, b, 1, a, 1);
        }

        template <typename DType>
        void cblas_sbmv(Shape& shape, DType* a, DType* b, DType* c, DType alpha = 1.0, DType beta = 1.0);

        template <> void cblas_sbmv(Shape& shape, float* a, float* b, float* c, float alpha, float beta)
        {
            cblas_ssbmv(CblasRowMajor, CblasLower, shape.prod(), 0, alpha, a, 1, b, 1, beta, c, 1);
        }

        template <> void cblas_sbmv(Shape& shape, double* a, double* b, double* c, double alpha, double beta)
        {
            cblas_dsbmv(CblasRowMajor, CblasLower, shape.prod(), 0, alpha, a, 1, b, 1, beta, c, 1);
        }


		template <typename DType>
		BLAS::Variable<DType>& operator+(BLAS::Variable<DType>& a, BLAS::Variable<DType>& b)
		{
			auto r = new BLAS::Variable<DType>(&a);

            // Sum
            cblas_axpy(a._shape, r->_values, b._values);

            // Adjoint
			Tape::current()->push([&a, &b, r]() {
                cblas_axpy(a._shape, a._adjs, r->_adjs);
                cblas_axpy(b._shape, b._adjs, r->_adjs);
			});

			return *r;
		}

		template <typename DType>
		BLAS::Variable<DType>& operator-(BLAS::Variable<DType>& a, BLAS::Variable<DType>& b)
		{
			auto r = new BLAS::Variable<DType>(&a);

            // Substract
            cblas_axpy(a._shape, r->_values, b._values, -1.0);

            // Adjoint
			Tape::current()->push([&a, &b, r]() {
                cblas_axpy(a._shape, a._adjs, r->_adjs);
                cblas_axpy(b._shape, b._adjs, r->_adjs, -1.0);
			});

			return *r;
		}

		template <typename DType>
		BLAS::Variable<DType>& operator*(BLAS::Variable<DType>& a, BLAS::Variable<DType>& b)
		{
			auto r = new BLAS::Variable<DType>(&a, 0, false);
            cblas_sbmv(a._shape, a._values, b._values, r->_values);

			DType* av = a._values;
			DType* bv = b._values;

			Tape::current()->push([av, bv, &a, &b, r]() {
                cblas_sbmv(a._shape, bv, r->_adjs, a._adjs);
                cblas_sbmv(a._shape, av, r->_adjs, b._adjs);

                /*
				SHAPE_LOOP(a._shape) {
					a._adjs[a._shape.idx(x, y)] += bv[a._shape.idx(x, y)] * r->_adjs[a._shape.idx(x, y)];
					b._adjs[a._shape.idx(x, y)] += av[a._shape.idx(x, y)] * r->_adjs[a._shape.idx(x, y)];
				}
                */
			});

			return *r;
		}

		template <typename DType>
		BLAS::Variable<DType>& operator/(BLAS::Variable<DType>& a, BLAS::Variable<DType>& b)
		{
			auto r = new BLAS::Variable<DType>(&a, 0, false);
            cblas_sbmv(a._shape, a._values, b._values, r->_values);

			// Do the sum
			SHAPE_LOOP(a._shape) {
				r->_values[a._shape.idx(x, y)] = a._values[a._shape.idx(x, y)] / b._values[a._shape.idx(x, y)];
			}

			DType* av = a._values;
			DType* bv = b._values;

			Tape::current()->push([av, bv, &a, &b, r]() {
				SHAPE_LOOP(a._shape) {
					auto bv2 = bv[a._shape.idx(x, y)] * bv[a._shape.idx(x, y)];

					a._adjs[a._shape.idx(x, y)] += (bv[a._shape.idx(x, y)] * r->_adjs[a._shape.idx(x, y)]) / bv2;
					b._adjs[a._shape.idx(x, y)] -= (av[a._shape.idx(x, y)] * r->_adjs[a._shape.idx(x, y)]) / bv2;
				}
			});

			return *r;
		}

		template <typename DType>
		BLAS::Variable<DType>& sqrt(BLAS::Variable<DType>& a)
		{
			auto r = new BLAS::Variable<DType>(a._shape);

			// Do the sum
			SHAPE_LOOP(a._shape) {
				r->_values[a._shape.idx(x, y)] = std::sqrt(a._values[a._shape.idx(x, y)]);
			}

			DType* rv = r->_values;
			DType* adjs = r->_adjs;

			Tape::current()->push([rv, &a, adjs]() {
				SHAPE_LOOP(a._shape) {
					a._adj[a._shape.idx(x, y)] += adjs[a._shape.idx(x, y)] / (DType(2.0) * rv[a._shape.idx(x, y)]);
				}
			});

			return *r;
		}

		template <typename DType, typename D>
		BLAS::Variable<DType>& pow(BLAS::Variable<DType>& a, D expo)
		{
			auto r = new BLAS::Variable<DType>(a._shape);

			// Do the sum
			SHAPE_LOOP(a._shape) {
				r->_values[a._shape.idx(x, y)] = std::pow(a._values[a._shape.idx(x, y)], expo);
			}

			DType* rv = r->_values;
			DType* adjs = r->_adjs;
			DType* av = a._values;

			Tape::current()->push([rv, &a, av, adjs, expo]() {
				SHAPE_LOOP(a._shape) {
					a._adjs[a._shape.idx(x, y)] += adjs[a._shape.idx(x, y)] * expo * std::pow(av[a._shape.idx(x, y)], DType(expo - 1));
				}
			});

			return *r;
		}
	}
}

namespace F32
{
	namespace BLAS
	{
		inline Bare::BLAS::Variable<float>& Variable(Shape shape, float value)
		{
			return *(new Bare::BLAS::Variable<float>(shape, value));
		}
	}
}

namespace F64
{
	namespace BLAS
	{
		inline Bare::BLAS::Variable<double>& Variable(Shape shape, double value)
		{
			return *(new Bare::BLAS::Variable<double>(shape, value));
		}
	}
}
