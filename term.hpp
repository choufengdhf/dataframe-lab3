#pragma once
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <sstream>
using namespace std;

struct term {
    double coefficient = 0.0;
    int degree = 0;
    // 构造函数接受 double，避免小数被截断
    term(double c = 0.0, int d = 0) : coefficient(c), degree(d) {}
    // 视为同类项的条件：仅比较 degree（便于合并同次数项）
    bool operator==(const term &other) const { return degree == other.degree; }
    // 降幂排列（degree 大的先输出）
    bool operator<(const term &other) const { return degree > other.degree; } // 降幂
    bool operator>(const term &other) const { return !((*this) == other) && !((*this) < other); }
    term(const term &other) = default;
    term& operator=(const term& other) {
        if (this != &other) {
            coefficient = other.coefficient;
            degree = other.degree;
        }
        return *this;
    }
    term operator+(const term &other) const {
        if (degree != other.degree) throw invalid_argument("Degrees are different");
        term res(*this);
        res.coefficient += other.coefficient;
        return res;
    }
    term operator-(const term &other) const {
        if (degree != other.degree) throw invalid_argument("Degrees are different");
        term res(*this);
        res.coefficient -= other.coefficient;
        return res;
    }
    term operator*(const term &other) const {
        term res(*this);
        res.coefficient *= other.coefficient;
        res.degree += other.degree;
        return res;
    }
    void differentiate(){
        coefficient *= static_cast<double>(degree);
        --degree;
    }
};

// 输出格式：接近整数按整数输出，否则按浮点输出（6位小数）
inline ostream& operator<<(ostream& os, const term& t) {
    const double eps = 1e-12;
    double c = t.coefficient;
    if (fabs(c) < eps) {
        os << "0x^" << t.degree;
        return os;
    }
    long long ic = llround(c);
    if(ic==1&&t.degree!=0) os<<""; // 系数为1且非常数项，省略系数
    else if(ic==-1&&t.degree!=0) os<<'-'; // 系数为-1且非常数项，省略系数但保留负号
    else if (fabs(c - (double)ic) < eps) {
        os << ic;
    } else {
        ostringstream ss;
        ss.setf(ios::fixed);
        ss.precision(6);
        ss << c;
        os << ss.str();
    }
    os << "x^" << t.degree;
    return os;
}

namespace std {
template<>
struct hash<term> {
    size_t operator()(const term& t) const {
        // operator== 比较 degree，因此 hash 也仅基于 degree
        return hash<int>()(t.degree);
    }
};
}