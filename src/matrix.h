#ifndef MATRIX_INCLUDED
#define MATRIX_INCLUDED

#include "src/cpu/cpu_types.h"
#include <functional>

template <typename T>
struct Matrix {
    Matrix(Word width, Word height);
    ~Matrix();

    T* array;
    Word width;
    Word height;

    void set(Word x, Word y, T value);
    T get(Word x, Word y);

    void for_each(std::function<void(Word, Word)> function);
    void copy(Matrix<T> * matrix);
};

#endif