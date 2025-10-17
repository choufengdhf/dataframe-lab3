#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cctype>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <map>
#include <iostream>
using namespace std;

namespace expr_eval {

using VarMap = unordered_map<string, double>;

// ExpressionEvaluator: tokenize, toRPN, evalRPN, evaluate, extractVariables
class ExpressionEvaluator {
public:
    static inline vector<string> tokenize(const string& expr);
    static inline vector<string> toRPN(const vector<string>& tokens);
    static inline double evalRPN(const vector<string>& rpn, const VarMap& varmap);
    static inline double evaluate(const string& expr, const VarMap& varmap);
    static inline vector<string> extractVariables(const string& expr);
};

// 标识符规则：首位必须是字母；后续可以是字母、数字或下划线
inline bool isIdentifierStart(char c) {
    return isalpha(static_cast<unsigned char>(c));
}
inline bool isIdentifierChar(char c) {
    return isalpha(static_cast<unsigned char>(c)) 
        || isdigit(static_cast<unsigned char>(c))
        || c == '_';
}

inline vector<string> ExpressionEvaluator::tokenize(const string& expr) {
    vector<string> tokens;
    size_t i = 0;
    while (i < expr.size()) {
        char c = expr[i];
        if (isspace(static_cast<unsigned char>(c))) { ++i; continue; }
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^' ||
            c == '(' || c == ')') {
            tokens.emplace_back(1, c);
            ++i;
            continue;
        }
        if (isdigit(static_cast<unsigned char>(c)) || c == '.') {
            size_t j = i;
            bool dotSeen = false;
            while (j < expr.size() && (isdigit((unsigned char)expr[j]) || expr[j] == '.')) {
                if (expr[j] == '.') {
                    if (dotSeen) break;
                    dotSeen = true;
                }
                ++j;
            }
            tokens.push_back(expr.substr(i, j - i));
            i = j;
            continue;
        }
        if (isIdentifierStart(c)) {
            size_t j = i + 1;
            while (j < expr.size() && isIdentifierChar(expr[j])) ++j;
            tokens.push_back(expr.substr(i, j - i));
            i = j;
            continue;
        }
        ostringstream oss;
        oss << "Invalid character in expression: '" << c << "'";
        throw invalid_argument(oss.str());
    }
    return tokens;
}

inline int precedenceOf(const string& op) {
    if (op == "u-") return 4; // unary minus highest
    if (op == "^") return 3;
    if (op == "*" || op == "/") return 2;
    if (op == "+" || op == "-") return 1;
    return 0;
}

inline bool isRightAssociative(const string& op) {
    if (op == "^") return true;
    if (op == "u-") return true;
    return false;
}

inline vector<string> ExpressionEvaluator::toRPN(const vector<string>& tokens) {
    vector<string> output;
    vector<string> ops;
    string prevToken = "";
    for (size_t i = 0; i < tokens.size(); ++i) {
        const string& tk = tokens[i];
        if (!tk.empty() && (isdigit((unsigned char)tk[0]) || tk[0] == '.')) {
            output.push_back(tk);
        }
        else if (!tk.empty() && isIdentifierStart(tk[0])) {
            output.push_back(tk);
        }
        else if (tk == "+" || tk == "-" || tk == "*" || tk == "/" || tk == "^") {
            string op = tk;
            // detect unary minus
            if (op == "-") {
                if (prevToken.empty() || prevToken == "(" ||
                    prevToken == "+" || prevToken == "-" ||
                    prevToken == "*" || prevToken == "/" || prevToken == "^") {
                    op = "u-";
                }
            }
            while (!ops.empty()) {
                string top = ops.back();
                if (top == "(") break;
                int p1 = precedenceOf(top);
                int p2 = precedenceOf(op);
                if ((isRightAssociative(op) && p2 < p1) ||
                    (!isRightAssociative(op) && p2 <= p1)) {
                    output.push_back(top);
                    ops.pop_back();
                } else break;
            }
            ops.push_back(op);
        }
        else if (tk == "(") {
            ops.push_back(tk);
        }
        else if (tk == ")") {
            bool found = false;
            while (!ops.empty()) {
                string top = ops.back(); ops.pop_back();
                if (top == "(") { found = true; break; }
                output.push_back(top);
            }
            if (!found) throw invalid_argument("Mismatched parentheses: no matching '(' for ')'");
        }
        else {
            throw invalid_argument("Unknown token when converting to RPN: " + tk);
        }
        prevToken = tk;
    }
    while (!ops.empty()) {
        string top = ops.back(); ops.pop_back();
        if (top == "(" || top == ")") throw invalid_argument("Mismatched parentheses in expression");
        output.push_back(top);
    }
    return output;
}

inline double ExpressionEvaluator::evalRPN(const vector<string>& rpn, const VarMap& varmap) {
    stack<double> st;
    for (const auto& tk : rpn) {
        if (tk.empty()) continue;

        // numbers
        if (isdigit((unsigned char)tk[0]) || tk[0] == '.') {
            try {
                double v = stod(tk);
                st.push(v);
                continue;
            } catch (...) {
                throw invalid_argument("Invalid number token: " + tk);
            }
        }

        // operators (handle "u-" before identifier check)
        if (tk == "u-") {
            if (st.empty()) throw invalid_argument("Missing operand for unary minus");
            double a = st.top(); st.pop();
            st.push(-a);
            continue;
        }
        if (tk == "+" || tk == "-" || tk == "*" || tk == "/" || tk == "^") {
            if (st.size() < 2) throw invalid_argument("Not enough operands for binary operator: " + tk);
            double b = st.top(); st.pop();
            double a = st.top(); st.pop();
            double res = 0;
            if (tk == "+") res = a + b;
            else if (tk == "-") res = a - b;
            else if (tk == "*") res = a * b;
            else if (tk == "/") {
                if (b == 0.0) throw invalid_argument("Division by zero");
                res = a / b;
            }
            else if (tk == "^") {
                res = pow(a, b);
            }
            st.push(res);
            continue;
        }

        // identifiers
        if (isIdentifierStart(tk[0])) {
            auto it = varmap.find(tk);
            if (it == varmap.end()) {
                throw invalid_argument("Unknown variable: " + tk);
            }
            st.push(it->second);
            continue;
        }

        // unknown
        throw invalid_argument("Unexpected token in RPN evaluation: " + tk);
    }
    if (st.size() != 1) throw invalid_argument("Invalid expression evaluation: stack has multiple values");
    return st.top();
}

inline double ExpressionEvaluator::evaluate(const string& expr, const VarMap& varmap) {
    auto tokens = tokenize(expr);
    auto rpn = toRPN(tokens);
    return evalRPN(rpn, varmap);
}

inline vector<string> ExpressionEvaluator::extractVariables(const string& expr) {
    auto tokens = tokenize(expr);
    unordered_set<string> s;
    for (auto &tk : tokens) {
        if (!tk.empty() && isIdentifierStart(tk[0])) {
            // ensure token strictly matches identifier rule
            bool ok = true;
            for (size_t i = 0; i < tk.size(); ++i) {
                char c = tk[i];
                if (i == 0) {
                    if (!isalpha(static_cast<unsigned char>(c))) { ok = false; break; }
                } else {
                    if (!(isalpha(static_cast<unsigned char>(c)) || isdigit(static_cast<unsigned char>(c)) || c == '_')) { ok = false; break; }
                }
            }
            if (ok) s.insert(tk);
        }
    }
    vector<string> res;
    res.reserve(s.size());
    for (auto &v : s) res.push_back(v);
    sort(res.begin(), res.end());
    return res;
}

// ---------------- Polynomial symbolic evaluation ----------------

struct Poly {
    map<int,double, greater<int>> coef; // degree -> coefficient (allows negative degrees)
    void addCoeff(int deg, double c) {
        coef[deg] += c;
        if (fabs(coef[deg]) < 1e-12) coef.erase(deg);
    }
    static Poly fromNumber(double v) {
        Poly p; if (fabs(v) > 1e-12) p.coef[0] = v; return p;
    }
    static Poly fromVar() {
        Poly p; p.coef[1] = 1.0; return p;
    }
    Poly operator+(const Poly& other) const {
        Poly r = *this;
        for (auto &kv : other.coef) r.addCoeff(kv.first, kv.second);
        return r;
    }
    Poly operator-(const Poly& other) const {
        Poly r = *this;
        for (auto &kv : other.coef) r.addCoeff(kv.first, -kv.second);
        return r;
    }
    Poly operator*(const Poly& other) const {
        Poly r;
        for (auto &a : coef) {
            for (auto &b : other.coef) {
                r.addCoeff(a.first + b.first, a.second * b.second);
            }
        }
        return r;
    }

    // integer exponent; negative exponent supported only when base is monomial/constant
    Poly pow_int(long long e) const {
        if (e == 0) return Poly::fromNumber(1.0);
        if (e > 0) {
            Poly base = *this;
            Poly res = Poly::fromNumber(1.0);
            long long exp = e;
            while (exp > 0) {
                if (exp & 1) res = res * base;
                base = base * base;
                exp >>= 1;
            }
            return res;
        } else { // e < 0
            if (coef.empty()) throw invalid_argument("Zero to negative power");
            if (coef.size() != 1) throw invalid_argument("Negative exponent only supported for monomial/constant base");
            auto kv = *coef.begin();
            int deg = kv.first;
            double a = kv.second;
            if (a == 0.0) throw invalid_argument("Zero to negative power");
            double a_pow = pow(a, (double)e); // e negative allowed
            Poly res;
            res.coef[deg * e] = a_pow;
            return res;
        }
    }
};

// Evaluate RPN into Poly (varName default "x")
inline Poly evalRPNtoPoly(const vector<string>& rpn, const string& varName = "x") {
    stack<Poly> st;
    for (const auto &tk : rpn) {
        if (tk.empty()) continue;

        // numbers
        if (isdigit((unsigned char)tk[0]) || tk[0] == '.') {
            double v = 0.0;
            try { v = stod(tk); } catch (...) { throw invalid_argument("Invalid number token in polynomial expression: " + tk); }
            st.push(Poly::fromNumber(v));
            continue;
        }

        // operators (handle "u-" first)
        if (tk == "u-") {
            if (st.empty()) throw invalid_argument("Missing operand for unary minus");
            Poly a = st.top(); st.pop();
            st.push(Poly::fromNumber(0.0) - a);
            continue;
        }
        if (tk == "+" || tk == "-" || tk == "*" || tk == "^") {
            if (st.size() < 2) throw invalid_argument("Not enough operands for operator in polynomial expression: " + tk);
            Poly b = st.top(); st.pop();
            Poly a = st.top(); st.pop();
            if (tk == "+") { st.push(a + b); continue; }
            if (tk == "-") { st.push(a - b); continue; }
            if (tk == "*") { st.push(a * b); continue; }
            if (tk == "^") {
                // exponent must be integer constant (degree 0)
                if (b.coef.empty()) { st.push(a.pow_int(0)); continue; }
                if (b.coef.size() == 1 && b.coef.begin()->first == 0) {
                    double dv = b.coef.begin()->second;
                    long long iv = (long long)llround(dv);
                    if (fabs(dv - (double)iv) > 1e-9) throw invalid_argument("Exponent must be integer constant for polynomial power");
                    st.push(a.pow_int(iv));
                    continue;
                }
                throw invalid_argument("Exponent must be integer constant for polynomial power");
            }
        }

        // identifiers (after operators)
        if (isIdentifierStart(tk[0])) {
            if (tk == varName) st.push(Poly::fromVar());
            else {
                ostringstream oss;
                oss << "Unsupported identifier '" << tk << "' in polynomial expression (only variable '" << varName << "' allowed)";
                throw invalid_argument(oss.str());
            }
            continue;
        }

        throw invalid_argument("Unexpected token in polynomial RPN: " + tk);
    }
    if (st.size() != 1) throw invalid_argument("Invalid polynomial expression: stack has multiple values");
    return st.top();
}

// Parse expression -> Poly
inline Poly parseExpressionToPoly(const string& expr, const string& varName = "x") {
    auto tokens = ExpressionEvaluator::tokenize(expr);
    auto rpn = ExpressionEvaluator::toRPN(tokens);
    Poly p = evalRPNtoPoly(rpn, varName);
    return p;
}

} // namespace expr_eval