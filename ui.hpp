#pragma once
#include <iostream>
#include <limits>
#include <string>
#include <cmath>
#include <sstream>

#include "SeqList.hpp"
#include "LinkList.hpp"
#include "evaluator.hpp"

using namespace std;
using namespace expr_eval;

// ===== utility =====
inline void clearCin() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// 读取并返回一整行（保留空格），若读取失败返回空字符串
inline string readLineFull() {
    string s;
    if (!getline(cin, s)) return string();
    return s;
}

// ===== menus =====
inline void printMainMenu() {
    cout << "\n========= 简易计算器 =========\n";
    cout << "请选择模块:\n";
    cout << "1. 一元多项式（顺序表）\n";
    cout << "2. 一元多项式（链表）\n";
    cout << "3. 向量（顺序表）\n";
    cout << "4. 向量（链表）\n";
    cout << "5. 顺序表（基本线性表）\n";
    cout << "6. 链表（基本线性表）\n";
    cout << "7. 四则运算表达式求值\n";
    cout << "0. 退出\n";
    cout << "请输入您的选择: ";
}

inline void printSeqPolyMenu() {
    cout << "\n--- 一元多项式（顺序表）功能菜单 ---\n";
    cout << "1. 创建多项式并命名（输入表达式，变量名首位字母，随后字母/数字/下划线）\n";
    cout << "2. 列出已创建多项式\n";
    cout << "3. 删除多项式\n";
    cout << "4. 多项式加法（选择已创建的两个多项式）\n";
    cout << "5. 多项式减法（选择已创建的两个多项式）\n";
    cout << "6. 多项式乘法（选择已创建的两个多项式）\n";
    cout << "7. 多项式求导（选择已创建的多项式）\n";
    cout << "8. 多项式赋值/求值（在某点的值）\n";
    cout << "0. 返回上一级\n";
    cout << "请输入您的选择: ";
}

inline void printLinkPolyMenu() {
    cout << "\n--- 一元多项式（链表）功能菜单 ---\n";
    cout << "1. 创建多项式并命名（输入表达式，变量名首位字母，随后字母/数字/下划线）\n";
    cout << "2. 列出已创建多项式\n";
    cout << "3. 删除多项式\n";
    cout << "4. 多项式加法（选择已创建的两个多项式）\n";
    cout << "5. 多项式减法（选择已创建的两个多项式）\n";
    cout << "6. 多项式乘法（选择已创建的两个多项式）\n";
    cout << "7. 多项式求导（选择已创建的多项式）\n";
    cout << "8. 多项式赋值/求值（在某点的值）\n";
    cout << "0. 返回上一级\n";
    cout << "请输入您的选择: ";
}

inline void printSeqVectMenu() {
    cout << "\n--- 向量（顺序表）功能菜单 ---\n";
    cout << "1. 计算模长\n";
    cout << "2. 向量点积\n";
    cout << "3. 计算夹角余弦\n";
    cout << "0. 返回上一级\n";
    cout << "请输入您的选择: ";
}

inline void printLinkVectMenu() {
    cout << "\n--- 向量（链表）功能菜单 ---\n";
    cout << "1. 计算模长\n";
    cout << "2. 向量点积\n";
    cout << "3. 计算夹角余弦\n";
    cout << "0. 返回上一级\n";
    cout << "请输入您的选择: ";
}

inline void printSeqListMenu() {
    cout << "\n--- 顺序表（基本线性表）功能菜单 ---\n";
    cout << "1. 创建顺序表\n";
    cout << "2. 插入元素\n";
    cout << "3. 遍历顺序表\n";
    cout << "4. 清空顺序表\n";
    cout << "5. 销毁顺序表\n";
    cout << "0. 返回上一级\n";
    cout << "请输入您的选择: ";
}

inline void printLinkListMenu() {
    cout << "\n--- 链表（基本线性表）功能菜单 ---\n";
    cout << "1. 创建链表\n";
    cout << "2. 插入元素\n";
    cout << "3. 遍历链表\n";
    cout << "4. 清空链表\n";
    cout << "5. 销毁链表\n";
    cout << "0. 返回上一级\n";
    cout << "请输入您的选择: ";
}

// ===== polynomial creation by expression =====
// 自动检测变量名（基于 ExpressionEvaluator::extractVariables），仅允许单变量
// 并把用户使用的变量名写入 Polynomial 对象（需要 Polynomial_* 支持 setVariableName）
inline Polynomial_Seq inputPolynomial_Seq() {
    Polynomial_Seq poly;
    cout << "请输入多项式表达式（例如：2*x^3 + 4*x - 5，变量名首位字母，随后字母/数字/下划线）:\n";
    string expr = readLineFull();
    if (expr.empty()) {
        // 处理前面可能有残留换行
        expr = readLineFull();
    }
    if (expr.empty()) {
        cout << "输入为空，返回空多项式。\n";
        return poly;
    }

    try {
        auto vars = ExpressionEvaluator::extractVariables(expr);
        if (vars.size() > 1) {
            cout << "解析失败：仅支持单变量表达式，但检测到多个标识符：";
            for (auto &v : vars) cout << v << " ";
            cout << "\n请仅使用单个变量名（首位字母，随后字母/数字/下划线）。\n";
            return poly;
        }
        string varName = "x";
        if (vars.size() == 1) varName = vars.front();

        Poly p = parseExpressionToPoly(expr, varName);
        for (auto &kv : p.coef) {
            int deg = kv.first;
            double coef = kv.second;
            if (fabs(coef) < 1e-12) continue;
            poly.insert(term(coef, deg));
        }
        poly.setVariableName(varName);
        poly.sort();
    } catch (const exception &e) {
        cout << "解析失败: " << e.what() << "\n";
    }
    return poly;
}

inline Polynomial_Link inputPolynomial_Link() {
    Polynomial_Link poly;
    cout << "请输入多项式表达式（例如：2*x^3 + 4*x - 5，变量名首位字母，随后字母/数字/下划线）:\n";
    string expr = readLineFull();
    if (expr.empty()) {
        expr = readLineFull();
    }
    if (expr.empty()) {
        cout << "输入为空，返回空多项式。\n";
        return poly;
    }

    try {
        auto vars = ExpressionEvaluator::extractVariables(expr);
        if (vars.size() > 1) {
            cout << "解析失败：仅支持单变量表达式，但检测到多个标识符：";
            for (auto &v : vars) cout << v << " ";
            cout << "\n请仅使用单个变量名（首位字母，随后字母/数字/下划线）。\n";
            return poly;
        }
        string varName = "x";
        if (vars.size() == 1) varName = vars.front();

        Poly p = parseExpressionToPoly(expr, varName);
        for (auto &kv : p.coef) {
            int deg = kv.first;
            double coef = kv.second;
            if (fabs(coef) < 1e-12) continue;
            poly.insert(term(coef, deg));
        }
        poly.setVariableName(varName);
    } catch (const exception &e) {
        cout << "解析失败: " << e.what() << "\n";
    }
    return poly;
}

// ===== vector input functions (used in calculator.cpp) =====

inline vect_Seq inputVect_Seq() {
    vect_Seq v;
    cout << "请输入向量长度: ";
    int n; cin >> n;
    while (n <= 0) {
        cout << "请输入正整数长度: ";
        cin >> n;
    }
    cout << "依次输入每个分量: ";
    for (int i = 0; i < n; ++i) {
        double x; cin >> x;
        v.push_back(x);
    }
    clearCin();
    return v;
}

inline vect_Link inputVect_Link() {
    vect_Link v;
    cout << "请输入向量长度: ";
    int n; cin >> n;
    while (n <= 0) {
        cout << "请输入正整数长度: ";
        cin >> n;
    }
    cout << "依次输入每个分量: ";
    for (int i = 0; i < n; ++i) {
        double x; cin >> x;
        v.tailinsert(x);
    }
    clearCin();
    return v;
}

// ===== basic SeqList<int> operations (used by calculator.cpp) =====

inline void createSeqList(SeqList<int>& sl) {
    sl.clean();
    cout << "请输入顺序表元素个数: ";
    int n; cin >> n;
    while (n < 0) {
        cout << "请输入非负整数: ";
        cin >> n;
    }
    cout << "请输入元素（以空格分隔）: ";
    for (int i = 0; i < n; ++i) {
        int x; cin >> x;
        sl.push_back(x);
    }
    cout << "创建成功！" << endl;
    clearCin();
}

inline void traverseSeqList(const SeqList<int>& sl) {
    cout << "顺序表元素:\n";
    for (int i = 0; i < sl.length(); ++i) {
        cout << sl.getElem(i) << " ";
    }
    cout << endl;
}

inline void clearSeqList(SeqList<int>& sl) {
    sl.clean();
    cout << "顺序表已清空！" << endl;
}

inline void destroySeqList(SeqList<int>& sl) {
    sl.Destroy();
    cout << "顺序表已销毁！" << endl;
}

// ===== basic LinkList<int> operations (used by calculator.cpp) =====

inline void createLinkList(LinkList<int>& ll) {
    ll.clear();
    cout << "请输入链表元素个数: ";
    int n; cin >> n;
    while (n < 0) {
        cout << "请输入非负整数: ";
        cin >> n;
    }
    cout << "请输入元素（以空格分隔）: ";
    for (int i = 0; i < n; ++i) {
        int x; cin >> x;
        ll.tailinsert(x);
    }
    cout << "创建成功！" << endl;
    clearCin();
}

inline void traverseLinkList(const LinkList<int>& ll) {
    cout << "链表元素: ";
    ll.print();
}

inline void clearLinkList(LinkList<int>& ll) {
    ll.clear();
    cout << "链表已清空！" << endl;
}

inline void destroyLinkList(LinkList<int>& ll) {
    ll.clear();
    cout << "链表已销毁！" << endl;
}