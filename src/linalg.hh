#pragma once

#include <cstddef>
#include <iostream>
#include <array>
#include <cmath>
#include <numbers>
#include <algorithm>
#include <vector>

template <size_t N>
using vec = std::array<double, N>;

template <size_t N, size_t M>
using mat = std::array<vec<M>, N>;

template <size_t N>
constexpr vec<N> operator+(vec<N> a, vec<N> b) {
    vec<N> result;
    for (size_t i = 0; i < N; ++i) {
        result[i] = a[i] + b[i];
    }
    return result;
}

template <size_t N>
constexpr vec<N> operator-(vec<N> a, vec<N> b) {
    vec<N> result;
    for (size_t i = 0; i < N; ++i) {
        result[i] = a[i] - b[i];
    }
    return result;
}

template <size_t N>
constexpr vec<N> operator*(double a, vec<N> b) {
    vec<N> result;
    for (size_t i = 0; i < N; ++i) {
        result[i] = a * b[i];
    }
    return result;
}

template <size_t N>
constexpr vec<N> operator*(vec<N> a, double b) {
    return b * a;
}

template <size_t N>
constexpr vec<N> operator/(vec<N> a, double b) {
    return (1.0 / b) * a;
}

template <size_t N>
constexpr double dot(vec<N> a, vec<N> b) {
    double result = 0.0;
    for (size_t i = 0; i < N; ++i) {
        result += a[i] * b[i];
    }
    return result;
}

template <size_t N>
constexpr vec<N> operator*(vec<N> a, vec<N> b) {
    vec<N> result;
    for (size_t i = 0; i < N; ++i) {
        result[i] = a[i] * b[i];
    }
    return result;
}

template <size_t N, size_t M>
constexpr vec<N> operator*(mat<N, M> a, vec<M> b) {
    vec<N> result = {};
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < M; ++j) {
            result[i] += a[i][j] * b[j];
        }
    }
    return result;
}

template <size_t N, size_t M, size_t K>
constexpr mat<N, K> operator*(mat<N, M> a, mat<M, K> b) {
    mat<N, K> result = {};
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < K; ++j) {
            for (size_t k = 0; k < M; ++k) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    return result;
}

template<size_t N>
constexpr double length(vec<N> v) {
    return std::sqrt(dot(v, v));
}

template<size_t N>
constexpr vec<N> normalize(vec<N> v) {
    return v / length(v);
}

template<size_t N, size_t M>
constexpr mat<M, N> transpose(mat<N, M> m) {
    mat<M, N> result = {};
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < M; ++j) {
            result[j][i] = m[i][j];
        }
    }
    return result;
}

static inline vec<4> to_homogeneus(vec<3> v) {
    return vec<4>{v[0], v[1], v[2], 1.0};
}

static inline vec<3> from_homogeneus(vec<4> v) {
    return vec<3>{v[0] / v[3], v[1] / v[3], v[2] / v[3]};
}

static inline vec<2> from_homogeneus(vec<3> v) {
    return vec<2>{v[0] / v[2], v[1] / v[2]};
}

static inline mat<4,4> scale(double factor) {
    return mat<4,4>{{
        {factor, 0.0, 0.0, 0.0},
        {0.0, factor, 0.0, 0.0},
        {0.0, 0.0, factor, 0.0},
        {0.0, 0.0, 0.0, 1.0}
    }};
}

static inline mat<4,4> rotate_along_y(double angle) {
    return mat<4,4>{{
        {std::cos(angle), 0.0, std::sin(angle), 0.0},
        {0.0, 1.0, 0.0, 0.0},
        {-std::sin(angle), 0.0, std::cos(angle), 0.0},
        {0.0, 0.0, 0.0, 1.0}
    }};
}

static inline mat<4,4> rotate_along_x(double angle) {
    return mat<4,4>{{
        {1.0, 0.0, 0.0, 0.0},
        {0.0, std::cos(angle), -std::sin(angle), 0.0},
        {0.0, std::sin(angle), std::cos(angle), 0.0},
        {0.0, 0.0, 0.0, 1.0}
    }};
}

static inline mat<4,4> translate(vec<3> offset) {
    return mat<4,4>{{
        {1.0, 0.0, 0.0, offset[0]},
        {0.0, 1.0, 0.0, offset[1]},
        {0.0, 0.0, 1.0, offset[2]},
        {0.0, 0.0, 0.0, 1.0}
    }};
}

static inline mat<3, 3> top_left_submatrix(mat<4, 4> m) {
    mat<3, 3> result;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++i) {
            result[i][j] = m[i][j];
        }
    }
    return result;
}

static inline vec<4> reflect(vec<4> ray, vec<4> normal) {
    return ray - 2 * dot(ray, normal) * normal;
}

namespace test {
    // Test vector addition.
    static_assert(
        vec<3>{1.0, 2.0, 3.0} + vec<3>{4.0, 5.0, 6.0} == vec<3>{5.0, 7.0, 9.0},
        "vector addition is incorrect"
    );

    // Test vector subtraction.
    static_assert(
        vec<3>{1.0, 2.0, 3.0} - vec<3>{4.0, 5.0, 6.0} == vec<3>{-3.0, -3.0, -3.0},
        "vector subtraction is incorrect"
    );

    // Test scalar multiplication.
    static_assert(
        2.0 * vec<3>{1.0, 2.0, 3.0} == vec<3>{2.0, 4.0, 6.0},
        "scalar multiplication is incorrect"
    );

    // Test scalar multiplication.
    static_assert(
        vec<3>{1.0, 2.0, 3.0} * 2.0 == vec<3>{2.0, 4.0, 6.0},
        "scalar multiplication is incorrect"
    );

    // Test scalar division.
    static_assert(
        vec<3>{2.0, 4.0, 6.0} / 2.0 == vec<3>{1.0, 2.0, 3.0},
        "scalar division is incorrect"
    );

    // Test dot product.
    static_assert(
        dot(vec<3>{1.0, 2.0, 3.0}, vec<3>{4.0, 5.0, 6.0}) == 32.0,
        "dot product of two vectors is incorrect"
    );

    // Test matrix-vector multiplication usig a 2x3 matrix.
    static_assert(
        mat<2, 3>{{
           {1.0, 2.0, 3.0},
           {4.0, 5.0, 6.0}
        }} * vec<3>{1.0, 2.0, 3.0} == vec<2>{14.0, 32.0},
        "matrix-vector multiplication is incorrect"
    );

    // Test matrix-matrix multiplication.
    static_assert(
        mat<3, 3>{{
           {1.0, 2.0, 3.0},
           {4.0, 5.0, 6.0},
           {7.0, 8.0, 9.0}
        }} * mat<3, 3>{{
           {1.0, 2.0, 3.0},
           {4.0, 5.0, 6.0},
           {7.0, 8.0, 9.0}
        }} == mat<3, 3>{{
           {30.0, 36.0, 42.0},
           {66.0, 81.0, 96.0},
           {102.0, 126.0, 150.0}
        }},
        "matrix-matrix multiplication is incorrect"
    );

    // Test transpose.
    static_assert(
        transpose(mat<3, 3>{{
           {1.0, 2.0, 3.0},
           {4.0, 5.0, 6.0},
           {7.0, 8.0, 9.0}
        }}) == mat<3, 3>{{
           {1.0, 4.0, 7.0},
           {2.0, 5.0, 8.0},
           {3.0, 6.0, 9.0}
        }},
        "transpose is incorrect"
    );
}