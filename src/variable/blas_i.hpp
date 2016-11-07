#pragma once

#ifdef WITH_BLAS_SUPPORT


#include <functional>
#include <vector>
#include <cmath>

#include <cblas.h>
#include <openvml.h>


template <typename DType>
void blas_add(const Shape& shape, DType* a, DType* b);

template <> void blas_add(const Shape& shape, float* a, float* b)
{
    vsAdd(shape.prod(), a, b, a);
}

template <> void blas_add(const Shape& shape, double* a, double* b)
{
    vdAdd(shape.prod(), a, b, a);
}


template <typename DType>
void blas_sub(const Shape& shape, DType* a, DType* b);

template <> void blas_sub(const Shape& shape, float* a, float* b)
{
    vsSub(shape.prod(), a, b, a);
}

template <> void blas_sub(const Shape& shape, double* a, double* b)
{
    vdSub(shape.prod(), a, b, a);
}


template <typename DType>
void blas_mul(const Shape& shape, DType* a, DType* b, DType* c);

template <> void blas_mul(const Shape& shape, float* a, float* b, float* c)
{
    vsAddmul(shape.prod(), a, b, c);
}

template <> void blas_mul(const Shape& shape, double* a, double* b, double* c)
{
    vdAddmul(shape.prod(), a, b, c);
}


template <typename DType>
void blas_div(const Shape& shape, DType* a, DType* b, DType* c);

template <> void blas_div(const Shape& shape, float* a, float* b, float* c)
{
    vsAdddiv(shape.prod(), a, b, c);
}

template <> void blas_div(const Shape& shape, double* a, double* b, double* c)
{
    vdAdddiv(shape.prod(), a, b, c);
}


template <typename DType>
void blas_pow(const Shape& shape, DType* a, DType b, DType* c);

template <> void blas_pow(const Shape& shape, float* a, float b, float* c)
{
    vsPowx(shape.prod(), a, &b, c);
}

template <> void blas_pow(const Shape& shape, double* a, double b, double* c)
{
    vdPowx(shape.prod(), a, &b, c);
}


template <typename DType>
void blas_matmul(const Shape& shapeA, DType* a, const Shape& shapeB, DType* b, DType* y, DType alpha = 1.0, DType beta = 1.0, bool transposeA = false, bool transposeB = false);

template <> void blas_matmul(const Shape& shapeA, float* a, const Shape& shapeB, float* b, float* y, float alpha, float beta, bool transposeA, bool transposeB)
{
    auto shape_a = transposeA ? shapeA.T() : shapeA;
    auto shape_b = transposeB ? shapeB.T() : shapeB;

    int lda = transposeA ? shape_a.m : shape_a.n;
    int ldb = transposeB ? shape_a.n : shape_b.n;

    cblas_sgemm(CblasRowMajor, transposeA ? CblasTrans : CblasNoTrans, transposeB ? CblasTrans : CblasNoTrans,
        shape_a.m, shape_b.n, shape_a.n, alpha, a, lda, b, ldb, beta, y, shape_b.n);
}

template <> void blas_matmul(const Shape& shapeA, double* a, const Shape& shapeB, double* b, double* y, double alpha, double beta, bool transposeA, bool transposeB)
{
    auto shape_a = transposeA ? shapeA.T() : shapeA;
    auto shape_b = transposeB ? shapeB.T() : shapeB;

    int lda = transposeA ? shape_a.m : shape_a.n;
    int ldb = transposeB ? shape_a.n : shape_b.n;

    cblas_dgemm(CblasRowMajor, transposeA ? CblasTrans : CblasNoTrans, transposeB ? CblasTrans : CblasNoTrans,
        shape_a.m, shape_b.n, shape_a.n, alpha, a, lda, b, ldb, beta, y, shape_b.n);
}

#endif
