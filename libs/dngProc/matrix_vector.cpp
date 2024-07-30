#include "matrix_vector.h"
#include <cstdlib>

using namespace std;


Matriz::Matriz(float a, float b, float c,
               float d, float e, float f,
               float g, float h, float i) {
    m = new float[9]{
            a, b, c,
            d, e, f,
            g, h, i,
    };
}

Matriz::Matriz() {
    m = new float[9]{
            0.f, 0.f, 0.f,
            0.f, 0.f, 0.f,
            0.f, 0.f, 0.f,
    };
}

Matriz::Matriz(const Matriz& mat)
{
    m = new float[9];
    for (int i = 0; i < 9; ++i) {
        m[i] = mat(i);
    }
}

Matriz::Matriz(float n) {
    m = new float[9]{
            n, n, n,
            n, n, n,
            n, n, n
    };
}

Matriz::Matriz(float *d) {
    m = new float[9];
    for (int i = 0; i < 9; ++i)
        m[i] = d[i];
}

Matriz::Matriz(double *d) {
    m = new float[9];
    for (int i = 0; i < 9; ++i)
        m[i] = (float) d[i];
}

Matriz::~Matriz() {
    delete[] m;
}

float &Matriz::operator()(int p) {
    return m[p];
}
float &Matriz::operator()(int x, int y) {
    return m[x + y * 3];
}

float &Matriz::operator()(int p) const {
    return m[p];
}
float &Matriz::operator()(int x, int y) const {
    return m[x + y * 3];
}

Matriz& Matriz::operator=(const Matriz& mat)
{
    Matriz &t = *this;
    for (int i = 0; i < 9; ++i) {
        t(i) = mat(i);
    }
    return *this;
}
Matriz& Matriz::operator=(double* mat)
{
    Matriz &t = *this;
    for (int i = 0; i < 9; ++i) {
        t(i) = (float)mat[i];
    }
    return *this;
}

Matriz Matriz::inverse() {
    Matriz &mat = *this;
    float det = mat(0, 0) * (mat(1, 1) * mat(2, 2) - mat(2, 1) * mat(1, 2)) -
                mat(0, 1) * (mat(1, 0) * mat(2, 2) - mat(1, 2) * mat(2, 0)) +
                mat(0, 2) * (mat(1, 0) * mat(2, 1) - mat(1, 1) * mat(2, 0));

    float invdet = 1 / det;

    Matriz minv;
    minv(0, 0) = (mat(1, 1) * mat(2, 2) - mat(2, 1) * mat(1, 2)) * invdet;
    minv(0, 1) = (mat(0, 2) * mat(2, 1) - mat(0, 1) * mat(2, 2)) * invdet;
    minv(0, 2) = (mat(0, 1) * mat(1, 2) - mat(0, 2) * mat(1, 1)) * invdet;
    minv(1, 0) = (mat(1, 2) * mat(2, 0) - mat(1, 0) * mat(2, 2)) * invdet;
    minv(1, 1) = (mat(0, 0) * mat(2, 2) - mat(0, 2) * mat(2, 0)) * invdet;
    minv(1, 2) = (mat(1, 0) * mat(0, 2) - mat(0, 0) * mat(1, 2)) * invdet;
    minv(2, 0) = (mat(1, 0) * mat(2, 1) - mat(2, 0) * mat(1, 1)) * invdet;
    minv(2, 1) = (mat(2, 0) * mat(0, 1) - mat(0, 0) * mat(2, 1)) * invdet;
    minv(2, 2) = (mat(0, 0) * mat(1, 1) - mat(1, 0) * mat(0, 1)) * invdet;
    return minv;
}

string Matriz::toStr() {
    Matriz mat = *this;
    string str = "";
    str += to_string(mat(0)) + " " + to_string(mat(1)) + " " + to_string(mat(2)) + "\n";
    str += to_string(mat(3)) + " " + to_string(mat(4)) + " " + to_string(mat(5)) + "\n";
    str += to_string(mat(6)) + " " + to_string(mat(7)) + " " + to_string(mat(8)) + "\n";
    return str;
}

Vec3::Vec3(float a, float b, float c) {
    m = new float[3]{
            a, b, c
    };
}

Vec3::Vec3() {
    m = new float[3]{
            0.f, 0.f, 0.f
    };
}

Vec3::Vec3(const Vec3& vec)
{
    m = new float[3]{
            vec(0), vec(1), vec(2)
    };
}

Vec3::Vec3(float n) {
    m = new float[3]{
            n, n, n
    };
}

Vec3::Vec3(float *m) {
    this->m = m;
}

Vec3::Vec3(double *d) {
    m = new float[3];
    m[0] = (float) d[0];
    m[1] = (float) d[1];
    m[2] = (float) d[2];
}

Vec3::~Vec3() {
    delete[] m;
}

float &Vec3::operator()(int p) {
    return m[p];
}
float &Vec3::operator()(int p) const {
    return m[p];
}

Vec3& Vec3::operator=(const Vec3& a) {
    Vec3 &v = *this;
    v(0) = a(0);
    v(1) = a(1);
    v(2) = a(2);
    return *this;
}

string Vec3::toStr() {
    Vec3 mat = *this;
    string str = "";
    str += to_string(mat(0)) + " " + to_string(mat(1)) + " " + to_string(mat(2)) + "\n";
    return str;
}

Vec3 operator*(const Vec3& v, const Matriz& m) {
    Vec3 out;
    out(0) = v(0) * m(0) + v(1) * m(1) + v(2) * m(2);
    out(1) = v(0) * m(3) + v(1) * m(4) + v(2) * m(5);
    out(2) = v(0) * m(6) + v(1) * m(7) + v(2) * m(8);
    return out;
}

Vec3 operator*(const Vec3& v, float n) {
    return {
            v(0) * n,
            v(1) * n,
            v(2) * n,
    };
}

Matriz operator*(const Matriz& a, const Matriz& b) {
    Matriz out;
    out(0) = a(0, 0) * b(0, 0) + a(1, 0) * b(0, 1) + a(2, 0) * b(6);
    out(1) = a(0, 0) * b(1, 0) + a(1, 0) * b(1, 1) + a(2, 0) * b(7);
    out(2) = a(0, 0) * b(2, 0) + a(1, 0) * b(2, 1) + a(2, 0) * b(8);
    out(3) = a(0, 1) * b(0, 0) + a(1, 1) * b(0, 1) + a(2, 1) * b(6);
    out(4) = a(0, 1) * b(1, 0) + a(1, 1) * b(1, 1) + a(2, 1) * b(7);
    out(5) = a(0, 1) * b(2, 0) + a(1, 1) * b(2, 1) + a(2, 1) * b(8);
    out(6) = a(0, 2) * b(0, 0) + a(1, 2) * b(0, 1) + a(2, 2) * b(6);
    out(7) = a(0, 2) * b(1, 0) + a(1, 2) * b(1, 1) + a(2, 2) * b(7);
    out(8) = a(0, 2) * b(2, 0) + a(1, 2) * b(2, 1) + a(2, 2) * b(8);
    return out;
}

Matriz operator*(const Matriz& a, float n) {
    Matriz out;
    for (int i = 0; i < 9; i++) {
        out(i) = a(i) * n;
    }
    return out;
}

//Matriz& mix(const Matriz& a, const Matriz& b, float f) {
Matriz mix(Matriz a, Matriz b, float f) {
    Matriz out;
    for (int i = 0; i < 9; i++)
        out(i) = (a(i) * f) + (b(i) * (1.f - f));
    return out;
}


Vec3 Vmap(const Matriz& matrix, const Vec3& input) {
    Vec3 output;
    output(0) = input(0) * matrix(0) + input(1) * matrix(1) + input(2) * matrix(2);
    output(1) = input(0) * matrix(3) + input(1) * matrix(4) + input(2) * matrix(5);
    output(2) = input(0) * matrix(6) + input(1) * matrix(7) + input(2) * matrix(8);
    return output;
}

float Vmax(const Vec3& array) {
    float val = array(0);
    for (int i = 0; i < 3; i++) {
        val = (array(i) > val) ? array(i) : val;
    }
    return val;
}

Vec3 max(const Vec3& x, float v) {
    Vec3 d;
    for (int i = 0; i < 3; i++) {
        d(i) = x(i) < v ? v : x(i);
    }
    return d;
}

Vec3 min(const Vec3& x, float v) {
    Vec3 d;
    for (int i = 0; i < 3; i++) {
        d(i) = x(i) > v ? v : x(i);
    }
    return d;
}

Vec3 clamp(const Vec3& x, float min, float max) {
    Vec3 d;
    for (int i = 0; i < 3; i++) {
        d(i) = x(i) < min ? min : x(i) > max ? max : x(i);
    }
    return d;
}

Vec3 clamp(const Vec3& x, float min, const Vec3& max) {
    Vec3 d;
    for (int i = 0; i < 3; i++) {
        d(i) = x(i) < min ? min : x(i) > max(i) ? max(i)
                                                : x(i);
    }
    return d;
}

Vec3 clamp(const Vec3& x, const Vec3& min_, float max) {
    Vec3 d;
    for (int i = 0; i < 3; i++) {
        d(i) = x(i) < min_(i) ? min_(i) : x(i) > max ? max
                                                     : x(i);
    }
    return d;
}

Vec3 clamp(const Vec3& x, const Vec3& min, const Vec3& max) {
    Vec3 d;
    for (int i = 0; i < 3; i++) {
        d(i) = x(i) < min(i) ? min(i) : x(i) > max(i) ? max(i)
                                                      : x(i);
    }
    return d;
}


