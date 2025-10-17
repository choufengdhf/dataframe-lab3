#pragma once
#include <iostream>
#include <unordered_set>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <cmath>
#include <string>
#include "term.hpp"
using namespace std;

template<typename Elemtype>
class SeqList {
protected:
    Elemtype* data = nullptr;
    int l = 0;
    int capacity = 128;
    unordered_set<Elemtype> s;

    void expand() {
        capacity *= 2;
        Elemtype* tmp = new Elemtype[capacity]();
        for (int i = 0; i < l; i++) {
            tmp[i] = data[i];
        }
        delete[] data;
        data = tmp;
    }

public:
    SeqList(int cap = 128) : capacity(cap) {
        data = new Elemtype[capacity]();
    }

    SeqList(Elemtype* da, int len) : l(0), capacity(max(128, len)) {
        data = new Elemtype[capacity]();
        for (int i = 0; i < len; i++) {
                push_back(da[i]);
        }
    }

    // 可变参数模板构造函数，只允许Elemtype类型参数，且忽略系数为0
    template<typename... Args,
        typename = std::enable_if_t<(std::is_same<Elemtype, typename std::decay<Args>::type>::value && ...)>>
    SeqList(Args&&... args) : capacity(128), l(0) {
        int n = sizeof...(args);
        while (capacity < n) this->expand();
        data = new Elemtype[capacity]();
        int dummy[] = { (this->push_back(std::forward<Args>(args)), 0)... };
        (void)dummy;
    }
    SeqList(const SeqList& other) : l(other.l), capacity(other.capacity), s(other.s) {
        data = new Elemtype[capacity]();
        for (int i = 0; i < l; i++) {
            data[i] = other.data[i];
        }
    }

    SeqList& operator=(const SeqList& other) {
        if (this == &other) return *this;
        delete[] data;
        capacity = other.capacity;
        l = other.l;
        s = other.s;
        data = new Elemtype[capacity]();
        for (int i = 0; i < l; i++) {
            data[i] = other.data[i];
        }
        return *this;
    }

    virtual ~SeqList() {
        delete[] data;
        data = nullptr;
    }

    SeqList operator+(const SeqList& other) const {
        if (l != other.l) throw invalid_argument("Different list length!");
        SeqList res(*this);
        for (int i = 0; i < l; i++) {
            res.data[i] = data[i] + other.data[i];
        }
        return res;
    }

    SeqList operator-(const SeqList& other) const {
        SeqList res(*this);
        for (int i = 0; i < l; i++) {
            res.data[i] = data[i] - other.data[i];
        }
        return res;
    }

    bool isEmpty() const { return l == 0; }

    void Destroy() {
        delete[] data;
        s.clear();
        data = nullptr;
        l = 0;
    }

    void initList() { clean(); }

    void clean() {
        delete[] data;
        data = new Elemtype[capacity]();
        l = 0;
        s.clear();
    }

    void push_back(Elemtype a) {
        if (l == capacity) this->expand();
        data[l++] = a;
        s.insert(a);
    }

    void Delete(int pos) {
        if (pos < 0 || pos >= l) return;
        s.erase(data[pos]);
        for (int i = pos; i < l - 1; i++) data[i] = data[i + 1];
        l--;
    }

    void insert(int pos, Elemtype value) {
        if (l == capacity) this->expand();
        for (int i = l - 1; i >= pos; i--) data[i + 1] = data[i];
        data[pos] = value;
        l++;
        s.insert(value);
    }

    int locate(Elemtype value, int defvalue = -1) const {
        if (!contains(value)) return -1;
        for (int i = 0; i < l; i++) {
            if (data[i] == value) return i;
        }
        return defvalue;
    }

    bool contains(const Elemtype& value) const {
        return s.count(value) > 0;
    }

    void Traverse(void(*visit)(Elemtype)) const {
        for (int i = 0; i < l; ++i) {
            visit(data[i]);
        }
    }

    void print() {
        for (int i = 0; i < l; i++) {
            cout << data[i] << endl;
        }
    }

    Elemtype getElem(int index) const { return data[index]; }
    Elemtype* begin() const { return data; }
    Elemtype* end() const { return data + l; }
    SeqList combine(const SeqList& other) const {
        SeqList res(*this);
        if (l + other.l > capacity) res.expand();
        for (int i = 0; i < other.l; i++) {
            auto elem = other.data[i];
            if (!res.contains(elem)) {
                res.push_back(elem);
                res.s.insert(elem);
            }
        }
        return res;
    }
    int length() const { return l; }
};

template<typename Elemtype>
class ordered_SeqList : public SeqList<Elemtype>
{
protected:
    using SeqList<Elemtype>::push_back;
    bool reverse = false;
public:
    ordered_SeqList(bool re = false, int cap = 128) : SeqList<Elemtype>(cap), reverse(re) {}

    // 可变参数模板构造函数，只允许Elemtype类型参数，且忽略系数为0
    template<typename... Args,
        typename = std::enable_if_t<(std::is_same<Elemtype, typename std::decay<Args>::type>::value && ...)>>
    ordered_SeqList(Args&&... args) : SeqList<Elemtype>(128) {
        int n = sizeof...(args);
        while (this->capacity < n) this->expand();
        this->l = 0;
        int dummy[] = { (this->push_back(std::forward<Args>(args)), 0)... };
        (void)dummy;
        this->sort();
    }

    ordered_SeqList(const SeqList<Elemtype>& other, bool rev = false) : SeqList<Elemtype>(other), reverse(rev) {
        this->sort();
    }

    ordered_SeqList& operator=(const ordered_SeqList& other) {
        if (this == &other) return *this;
        SeqList<Elemtype>::operator=(other);
        reverse = other.reverse;
        return *this;
    }

    void sort() { sort(reverse); }
    void sort(bool rev) {
        std::sort(this->data, this->data + this->l, std::less<Elemtype>());
    }

    int quickLocate(Elemtype value, int l, int r) const {
        int left = l, right = r;
        while (left < right) {
            int mid = left + (right - left) / 2;
            if (this->data[mid] < value)
                left = mid + 1;
            else
                right = mid;
        }
        return left;
    }

    void insert(Elemtype value) {
        if (this->contains(value)) throw invalid_argument("This value has been contained!");
        int pos = quickLocate(value, 0, this->l);
        SeqList<Elemtype>::insert(pos, value);
    }

    Elemtype getElem(int index) const { return this->data[index]; }
    Elemtype* begin() const { return this->data; }
    Elemtype* end() const { return this->data + this->l; }

    ordered_SeqList combine(const ordered_SeqList& other, bool rev = false) const {
        if (this->reverse != other.reverse) {
            const_cast<ordered_SeqList*>(this)->sort(!this->reverse);
        }
        ordered_SeqList res(rev, this->capacity + other.capacity);
        int i1 = 0, i2 = 0;
        while (i1 != this->l || i2 != other.l) {
            if (i1 == this->l)
                res.push_back(other.data[i2++]);
            else if (i2 == other.l)
                res.push_back(this->data[i1++]);
            else if (this->data[i1] < other.data[i2])
                res.push_back(this->data[i1++]);
            else
                res.push_back(other.data[i2++]);
        }
        res.sort(rev);
        return res;
    }
};


class Polynomial_Seq : public ordered_SeqList<term>
{
private:
    const bool reverse = 1;
    std::string variableName = "x"; // 记录用户输入的变量名，默认 x
    static constexpr double EPS = 1e-12;

    static std::string formatCoefficient(double c) {
        long long ic = llround(c);
        if (std::fabs(c - (double)ic) < EPS) {
            return std::to_string(ic);
        } else {
            std::ostringstream ss;
            ss.setf(std::ios::fixed);
            ss.precision(6);
            ss << c;
            return ss.str();
        }
    }

public:
    Polynomial_Seq() : ordered_SeqList<term>(true) {}

    // 可变参数模板构造函数，只允许term类型参数，且忽略系数为0
    template<typename... Args,
        typename = std::enable_if_t<(std::is_same<term, typename std::decay<Args>::type>::value && ...)>>
    Polynomial_Seq(Args&&... args)
        : ordered_SeqList<term>(true) {
        int dummy[] = { (args.coefficient != 0 && (this->push_back(std::forward<Args>(args)), true), 0)... };
        (void)dummy;
        this->sort();
    }

    Polynomial_Seq(term* d, int len) : ordered_SeqList<term>(true, max(len, 128)) {
        for (int i = 0; i < len; i++) {
            if (d[i].coefficient != 0)
                this->push_back(d[i]);
        }
        this->sort();
    }

    Polynomial_Seq(const Polynomial_Seq& other) : ordered_SeqList<term>(other, true), variableName(other.variableName) {}

    void setVariableName(const std::string& name) { variableName = name; }
    std::string getVariableName() const { return variableName; }

    void insert(term value) {
        // Allow inserting terms even if another term with same degree exists.
        // Ignore near-zero coefficients.
        if (std::fabs(value.coefficient) < EPS) return;
        int pos = this->quickLocate(value, 0, this->l);
        SeqList<term>::insert(pos, value);
    }

    Polynomial_Seq& operator=(const Polynomial_Seq& other) {
        if (this == &other) return *this;
        ordered_SeqList<term>::operator=(other);
        this->variableName = other.variableName;
        return *this;
    }

    term getElem(int index) const { return this->data[index]; }
    term* begin() const { return this->data; }
    term* end() const { return this->data + this->l; }

    Polynomial_Seq operator+(const Polynomial_Seq& other) const {
        Polynomial_Seq res;
        res.variableName = this->variableName; // adopt left's variable name by default
        int i1 = 0, i2 = 0;
        while (i1 != this->l || i2 != other.l) {
            if (i1 == this->l)
                res.push_back(other.data[i2++]);
            else if (i2 == other.l)
                res.push_back(this->data[i1++]);
            else if (this->data[i1].degree == other.data[i2].degree) {
                if (this->data[i1].coefficient + other.data[i2].coefficient == 0)
                    i1++, i2++;
                else
                    res.push_back(this->data[i1++] + other.data[i2++]);
            } else if (this->data[i1] < other.data[i2])
                res.push_back(this->data[i1++]);
            else
                res.push_back(other.data[i2++]);
        }
        return res;
    }

    Polynomial_Seq operator-(const Polynomial_Seq& other) const {
        Polynomial_Seq res;
        res.variableName = this->variableName;
        int i1 = 0, i2 = 0;
        while (i1 != this->l || i2 != other.l) {
            if (i1 == this->l)
                res.push_back(other.data[i2++]);
            else if (i2 == other.l)
                res.push_back(this->data[i1++]);
            else if (this->data[i1].degree == other.data[i2].degree) {
                if (this->data[i1].coefficient - other.data[i2].coefficient == 0)
                    i1++, i2++;
                else
                    res.push_back(this->data[i1++] - other.data[i2++]);
            } else if (this->data[i1] < other.data[i2])
                res.push_back(this->data[i1++]);
            else{
                term tmp=other.data[i2++];
                tmp.coefficient=-tmp.coefficient;
                res.push_back(tmp);
            }
        }
        return res;
    }

    Polynomial_Seq operator*(const Polynomial_Seq& other) const {
        Polynomial_Seq res;
        res.variableName = this->variableName;
        for (term* p1 = this->data; p1 != this->data + this->l; ++p1) {
            for (term* p2 = other.data; p2 != other.data + other.l; ++p2) {
                res.insert((*p1) * (*p2));
            }
        }
        for (int i = 0; i < res.l - 1; i++) {
            while (i < res.l - 1 && res.data[i].degree == res.data[i + 1].degree) {
                res.data[i] = res.data[i] + res.data[i + 1];
                res.Delete(i + 1);
            }
        }
        return res;
    }
    void differentiate(int num) {
        if(num<0) throw invalid_argument("The number of derivatives is less than 0");
        if(num==0) return ;
        for (int i = 0; i < this->l; i++) {
            for(int j=0;j<num;j++)
                this->data[i].differentiate();
            if (std::fabs(this->data[i].coefficient) < EPS ) {
                Delete(i);
                i--;
            }
        }
    }
    void print() {
        if (this->l == 0) {
            cout << "0" << endl;
            return;
        }
        auto printTerm = [&](const term &t, bool first) {
            double c = t.coefficient;
            int deg = t.degree;
            string cs = formatCoefficient(c);
            if (first) {
                cout << cs;
            } else {
                if (c >= 0) cout << '+';
                cout << cs;
            }
            cout << variableName << "^" << deg;
        };

        printTerm(this->data[0], true);
        for (int i = 1; i < this->l; i++) {
            printTerm(this->data[i], false);
        }
        cout << endl;
    }
};


class vect_Seq :public SeqList<double>{
    public:
    using SeqList<double>::SeqList;
    double module()const{
        double res=0;
        for(int i=0;i<l;i++){
            res+=data[i]*data[i];
        }
        return sqrt(res);
    }
    double operator*(const vect_Seq&other)const{
        if(l!=other.l) throw invalid_argument("Different vect lenths");
        double res=0;
        for(int i=0;i<l;i++){
            res+=data[i]*other.data[i];
        }
        return res;
    }
    
};

double cos(vect_Seq s1,vect_Seq s2){
        return (s1*s2)/(s1.module()*s2.module());
}