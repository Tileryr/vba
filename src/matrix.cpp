#include <stdlib.h>
#include <cstring>
#include "src/matrix.h"

template class Matrix<HalfWord>;

template<typename T>
Matrix<T>::Matrix(Word width, Word height) : width(width), height(height) {
    array = new T[width*height];
}

template<typename T>
Matrix<T>::~Matrix() {
    delete array;
}

template<typename T>
void Matrix<T>::set(Word x, Word y, T value) {
    array[y*width+x] = value;
}

template<typename T>
T Matrix<T>::get(Word x, Word y) {
    return array[y*width+x];
}

template<typename T>
void Matrix<T>::for_each(std::function<void(Word, Word)> function) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            function(x, y);
        }
    }
}

template<typename T>
void Matrix<T>::copy(Matrix<T> * matrix) {
    memcpy(array, matrix->array, sizeof(T)*width*height);
}