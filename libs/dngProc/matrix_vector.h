#ifndef MATH
#define MATH


#include <iostream>
#include <string>

using namespace std;

class Matriz {
private:
    float *m;

public:
    Matriz();
    Matriz(float a, float b, float c,
           float d, float e, float f,
           float g, float h, float i);

    Matriz(const Matriz& m);
    Matriz(float n);
    Matriz(float *m);
    Matriz(double *d);

    ~Matriz();

    float &operator()(int p);
    float &operator()(int x, int y);
    float &operator()(int p) const;
    float &operator()(int x, int y) const;

    Matriz& operator=(const Matriz& mat);
    Matriz& operator=(double* mat);

    Matriz inverse();

    string toStr();
};

class Vec3 {
private:
    float *m;

public:

    Vec3(float a, float b, float c);

    Vec3();

    Vec3(const Vec3& vec);

    Vec3(float n);

    Vec3(float *m);

    Vec3(double *d);

    ~Vec3();

    float &operator()(int p);
    float &operator()(int p) const;

    Vec3& operator=(const Vec3& a);

    string toStr();
};

Vec3 operator*(const Vec3& v, const Matriz& m);
Vec3 operator*(const Vec3& v, float n);
Matriz operator*(const Matriz& a, const Matriz& b);
Matriz operator*(const Matriz& a, float n);
Matriz mix(Matriz a, Matriz b, float f);

Vec3 Vmap(const Matriz& matrix, const Vec3& input);
float Vmax(const Vec3& array);
Vec3 max(const Vec3& x, float v);
Vec3 min(const Vec3& x, float v);
Vec3 clamp(const Vec3& x, float min, float max);
Vec3 clamp(const Vec3& x, float min, const Vec3& max);
Vec3 clamp(const Vec3& x, const Vec3& min_, float max);
Vec3 clamp(const Vec3& x, const Vec3& min, const Vec3& max);


#endif // MATH



/*



#pragma once
#ifndef MATH
#define MATH

#include "cstdlib"
#include "iostream"
#include <string>

using namespace std;
class Matriz;
class Vec3;

class Matriz
{
private:
    double *m;

public:
    Matriz(double a, double b, double c,
           double d, double e, double f,
           double g, double h, double i);

    Matriz();
    Matriz(double n);
    Matriz(double *m);
    Matriz(float *m);
    Matriz(double **m);

    double &operator()(int p);
    double &operator()(int p) const;
    double &operator()(int x, int y);
    double &operator()(int x, int y) const;
    Matriz operator+(Matriz b);
    Matriz operator+(Matriz b) const;
    Matriz operator*(Matriz b);
    Matriz operator*(Matriz b) const;
    Vec3 operator*(Vec3 b);
    Vec3 operator*(Vec3 b) const;
    Matriz operator*(double n);
    Matriz operator*(double n) const;

    Matriz inverse() const;
    static Matriz mix(Matriz a, Matriz b, double f);
    static double max(Matriz array);
    static Matriz clamp(Matriz x, double min, double max);

    // Converters
    static double *float2double(float *m);
    static double *float2double(float **m);
    static double *double2double(double **m);

    string toStr();
};

class Vec3
{
private:
    double *m;

public:
    Vec3(double a, double b, double c);
    Vec3();
    Vec3(double n);
    Vec3(double *m);
    Vec3(float *m);
    void release();
    double &operator()(int p);
    double &operator()(int p) const;
    Vec3 operator*(Matriz b);
    Vec3 operator*(Matriz b) const;
    Vec3 operator*(double n);
    Vec3 operator*(double n) const;
    void operator*=(Matriz &b);
    void operator*=(double n);
    Vec3 &operator=(Vec3 a);
    static Vec3 map(Matriz matrix, Vec3 input);
    static double max(Vec3 array);
    static Vec3 clamp(Vec3 x, double min, double max);
    static Vec3 clamp(Vec3 x, double min, Vec3 max);
    static Vec3 clamp(Vec3 x, Vec3 min, double max);
    static Vec3 clamp(Vec3 x, Vec3 min, Vec3 max);
    void clamp(double min, double max);
    void clamp(double min, Vec3 max);
    void clamp(Vec3 min, double max);
    void clamp(Vec3 min, Vec3 max);
    float *toFloatArray();
    // Converters
    static double *float2d(float *m);
    string toStr();
};

// #include "mathMatriz.h"
// #include "mathVector.h"

Matriz::Matriz(double a, double b, double c,
               double d, double e, double f,
               double g, double h, double i) : m(new double[9]{
                                                   a,
                                                   b,
                                                   c,
                                                   d,
                                                   e,
                                                   f,
                                                   g,
                                                   h,
                                                   i,
                                               })
{
}

Matriz::Matriz() : m(new double[9]{
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                   })
{
}

Matriz::Matriz(double n) : m(new double[9]{
                               n, n, n,
                               n, n, n,
                               n, n, n}) {}

Matriz::Matriz(double *m) : m(m) {}

Matriz::Matriz(float *m) : m(float2double(m)) {}

Matriz::Matriz(double **m) : m(double2double(m)) {}

double &Matriz::operator()(int p)
{
    return m[p];
}

double &Matriz::operator()(int p) const
{
    return m[p];
}

double &Matriz::operator()(int x, int y)
{
    return m[x + y * 3];
}

double &Matriz::operator()(int x, int y) const
{
    return m[x + y * 3];
}

Matriz Matriz::operator+(Matriz b)
{
    Matriz out;
    for (int i = 0; i < 3; i++)
    {
        out(i) = m[i] + b(i);
    }
    return out;
}

Matriz Matriz::operator+(Matriz b) const
{
    Matriz out;
    for (int i = 0; i < 3; i++)
    {
        out(i) = m[i] + b(i);
    }
    return out;
}

Matriz Matriz::operator*(Matriz b)
{
    Matriz a = *this;
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

Matriz Matriz::operator*(Matriz b) const
{
    Matriz a = *this;
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

Vec3 Matriz::operator*(Vec3 a)
{
    Matriz b = *this;
    Vec3 out;
    out(0) = a(0) * b(0) + a(1) * b(1) + a(2) * b(2);
    out(1) = a(0) * b(3) + a(1) * b(4) + a(2) * b(5);
    out(2) = a(0) * b(6) + a(1) * b(7) + a(2) * b(8);
    return out;
}

Vec3 Matriz::operator*(Vec3 a) const
{
    Matriz b = *this;
    Vec3 out;
    out(0) = a(0) * b(0) + a(1) * b(1) + a(2) * b(2);
    out(1) = a(0) * b(3) + a(1) * b(4) + a(2) * b(5);
    out(2) = a(0) * b(6) + a(1) * b(7) + a(2) * b(8);
    return out;
}

Matriz Matriz::operator*(double n)
{
    Matriz a = *this;
    Matriz out;
    for (int i = 0; i < 9; i++)
    {
        out(i) = a(i) * n;
    }
    return out;
}

Matriz Matriz::operator*(double n) const
{
    Matriz a = *this;
    Matriz out;
    for (int i = 0; i < 9; i++)
    {
        out(i) = a(i) * n;
    }
    return out;
}

Matriz Matriz::inverse() const
{
    const Matriz &mat = *this;
    // computes the inverse of a matrix m
    double det = mat(0, 0) * (mat(1, 1) * mat(2, 2) - mat(2, 1) * mat(1, 2)) -
                 mat(0, 1) * (mat(1, 0) * mat(2, 2) - mat(1, 2) * mat(2, 0)) +
                 mat(0, 2) * (mat(1, 0) * mat(2, 1) - mat(1, 1) * mat(2, 0));

    double invdet = 1 / det;

    Matriz minv(new double[9]); // inverse of matrix m
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

Matriz Matriz::mix(Matriz a, Matriz b, double f)
{
    Matriz out;
    for (int i = 0; i < 9; i++)
    {
        out(i) = (a(i) * f) + (b(i) * (1.0 - f));
    }
    return out;
}

double Matriz::max(Matriz array)
{
    double val = array.m[0];
    for (int i = 0; i < 9; ++i)
    {
        val = (array.m[i] > val) ? array.m[i] : val;
    }
    return val;
}

Matriz Matriz::clamp(Matriz x, double min, double max)
{
    double *mat = new double[9];
    for (int i = 0; i < 9; i++)
    {
        mat[i] = x.m[i] < min ? min : x.m[i] > max ? max
                                                   : x.m[i];
    }
    return Matriz(mat);
}

// Converters
double *Matriz::float2double(float *m)
{
    double *mat = (double *)malloc(9 * sizeof(double));
    for (int i = 0; i < 9; i++)
    {
        mat[i] = static_cast<double>(m[i]);
    }
    return mat;
}
double *Matriz::float2double(float **m)
{
    double *mat = (double *)malloc(9 * sizeof(double));
    mat[0] = m[0][0];
    mat[1] = m[0][1];
    mat[2] = m[0][2];
    mat[3] = m[1][0];
    mat[4] = m[1][1];
    mat[5] = m[1][2];
    mat[6] = m[2][0];
    mat[7] = m[2][1];
    mat[8] = m[2][2];
    return mat;
}
double *Matriz::double2double(double **m)
{
    return new double[9]{
        m[0][0],
        m[0][1],
        m[0][2],
        m[1][0],
        m[1][1],
        m[1][2],
        m[2][0],
        m[2][1],
        m[2][2],
    };
}

string Matriz::toStr()
{
    Matriz mat = *this;
    string str = "";
    str += to_string(mat(0)) + " " + to_string(mat(1)) + " " + to_string(mat(2)) + "\n";
    str += to_string(mat(3)) + " " + to_string(mat(4)) + " " + to_string(mat(5)) + "\n";
    str += to_string(mat(6)) + " " + to_string(mat(7)) + " " + to_string(mat(8)) + "\n";
    return str;
}

//
//
//

Vec3::Vec3(double a, double b, double c) : m(new double[3]{a, b, c}) {}
Vec3::Vec3() : m(new double[3]{0, 0, 0}) {}

Vec3::Vec3(double n) : m(new double[3]{n, n, n}) {}
Vec3::Vec3(double *m) : m(m) {}

Vec3::Vec3(float *m) : m(float2d(m)) {}

void Vec3::release()
{
    free(m);
}

double &Vec3::operator()(int p)
{
    return m[p];
}

double &Vec3::operator()(int p) const
{
    return m[p];
}

Vec3 Vec3::operator*(Matriz b)
{
    Vec3 a = *this;
    Vec3 out;
    out(0) = a(0) * b(0) + a(1) * b(1) + a(2) * b(2);
    out(1) = a(0) * b(3) + a(1) * b(4) + a(2) * b(5);
    out(2) = a(0) * b(6) + a(1) * b(7) + a(2) * b(8);
    return out;
}

Vec3 Vec3::operator*(Matriz b) const
{
    Vec3 a = *this;
    Vec3 out;
    out(0) = a(0) * b(0) + a(1) * b(1) + a(2) * b(2);
    out(1) = a(0) * b(3) + a(1) * b(4) + a(2) * b(5);
    out(2) = a(0) * b(6) + a(1) * b(7) + a(2) * b(8);
    return out;
    //        Vec3 out = new Vec3();
    //        out.i(0,b.i(0, 0) * i(0) + b.i(0, 1) * i(1) + b.i(0, 2) * i(2));
    //        out.i(1,b.i(1, 0) * i(0) + b.i(1, 1) * i(1) + b.i(1, 2) * i(2));
    //        out.i(2,b.i(2, 0) * i(0) + b.i(2, 1) * i(1) + b.i(2, 2) * i(2));
    //        return out;
}

Vec3 Vec3::operator*(double n)
{
    return Vec3(new double[3]{
        m[0] * n,
        m[1] * n,
        m[2] * n,
    });
}

Vec3 Vec3::operator*(double n) const
{
    return Vec3(new double[3]{
        m[0] * n,
        m[1] * n,
        m[2] * n,
    });
}

void Vec3::operator*=(Matriz &b)
{
    Vec3 &a = *this;
    Vec3 out;
    out(0) = a(0) * b(0) + a(1) * b(1) + a(2) * b(2);
    out(1) = a(0) * b(3) + a(1) * b(4) + a(2) * b(5);
    out(2) = a(0) * b(6) + a(1) * b(7) + a(2) * b(8);
    a = out;
    out.release();
}

void Vec3::operator*=(double n)
{
    Vec3 &t = *this;
    t(0) = t(0) * n;
    t(1) = t(1) * n;
    t(2) = t(2) * n;
}

Vec3 &Vec3::operator=(Vec3 a)
{
    Vec3 &v = *this;
    v(0) = a(0);
    v(1) = a(1);
    v(2) = a(2);
    return v;
}

Vec3 Vec3::map(Matriz matrix, Vec3 input)
{
    Vec3 output;
    output(0) = input(0) * matrix(0) + input(1) * matrix(1) + input(2) * matrix(2);
    output(1) = input(0) * matrix(3) + input(1) * matrix(4) + input(2) * matrix(5);
    output(2) = input(0) * matrix(6) + input(1) * matrix(7) + input(2) * matrix(8);
    return output;
}

double Vec3::max(Vec3 array)
{
    double val = array(0);
    for (int i = 0; i < 3; i++)
    {
        val = (array(i) > val) ? array(i) : val;
    }
    return val;
}

Vec3 Vec3::clamp(Vec3 x, double min, double max)
{
    Vec3 d = Vec3();
    for (int i = 0; i < 3; i++)
    {
        d(i) = x(i) < min ? min : x(i) > max ? max
                                             : x(i);
    }
    return d;
}

Vec3 Vec3::clamp(Vec3 x, double min, Vec3 max)
{
    Vec3 d = Vec3();
    for (int i = 0; i < 3; i++)
    {
        d(i) = x(i) < min ? min : x(i) > max(i) ? max(i)
                                                : x(i);
    }
    return d;
}

Vec3 Vec3::clamp(Vec3 x, Vec3 min, double max)
{
    Vec3 d = Vec3();
    for (int i = 0; i < 3; i++)
    {
        d(i) = x(i) < min(i) ? min(i) : x(i) > max ? max
                                                   : x(i);
    }
    return d;
}

Vec3 Vec3::clamp(Vec3 x, Vec3 min, Vec3 max)
{
    Vec3 d = Vec3();
    for (int i = 0; i < 3; i++)
    {
        d(i) = x(i) < min(i) ? min(i) : x(i) > max(i) ? max(i)
                                                      : x(i);
    }
    return d;
}

void Vec3::clamp(double min, double max)
{
    for (int i = 0; i < 3; i++)
    {
        operator()(i) = operator()(i) < min ? min : operator()(i) > max ? max
                                                                        :
                                                                        operator()(i);
    }
}

void Vec3::clamp(double min, Vec3 max)
{
    for (int i = 0; i < 3; i++)
    {
        operator()(i) = operator()(i) < min ? min : operator()(i) > max(i) ? max(i)
                                                                           :
                                                                           operator()(i);
    }
}

void Vec3::clamp(Vec3 min, double max)
{
    for (int i = 0; i < 3; i++)
    {
        operator()(i) = operator()(i) < min(i) ? min(i) : operator()(i) > max ? max
                                                                              :
                                                                              operator()(i);
    }
}

void Vec3::clamp(Vec3 min, Vec3 max)
{
    for (int i = 0; i < 3; i++)
    {
        operator()(i) = operator()(i) < min(i) ? min(i) : operator()(i) > max(i) ? max(i)
                                                                                 :
                                                                                 operator()(i);
    }
}

float *Vec3::toFloatArray()
{
    Vec3 &vec = *this;
    return new float[3]{
        static_cast<float>(vec(0)),
        static_cast<float>(vec(1)),
        static_cast<float>(vec(2)),
    };
}
// Converters
double *Vec3::float2d(float *m)
{
    double *mat = (double *)malloc(3 * sizeof(double));
    for (int i = 0; i < 3; i++)
    {
        mat[i] = (double)m[i];
    }
    return mat;
}

string Vec3::toStr()
{
    Vec3 mat = *this;
    string str = "";
    str += to_string(mat(0)) + " " + to_string(mat(1)) + " " + to_string(mat(2)) + "\n";
    return str;
}

#endif // MATH

*/