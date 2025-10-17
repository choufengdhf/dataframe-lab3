#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

#include "evaluator.hpp"
#include "SeqList.hpp"
#include "LinkList.hpp"
#include "term.hpp"

using namespace std;
using namespace expr_eval;

const double EPS = 1e-9;

// Evaluate a Polynomial_Seq at x
static double evalPolySeq(const Polynomial_Seq &p, double x) {
    double res = 0.0;
    for (int i = 0; i < p.length(); ++i) {
        term t = p.getElem(i);
        res += t.coefficient * pow(x, t.degree);
    }
    return res;
}

// Evaluate a Polynomial_Link at x
static double evalPolyLink(const Polynomial_Link &p, double x) {
    double res = 0.0;
    for (auto t : p) {
        res += t.coefficient * pow(x, t.degree);
    }
    return res;
}

// Convert expr_eval::Poly -> Polynomial_Seq
static Polynomial_Seq polyToSeq(const Poly &p) {
    Polynomial_Seq ps;
    for (auto &kv : p.coef) {
        int deg = kv.first;
        double coef = kv.second;
        if (fabs(coef) < 1e-12) continue;
        ps.insert(term(coef, deg));
    }
    ps.sort();
    return ps;
}

// Convert expr_eval::Poly -> Polynomial_Link
static Polynomial_Link polyToLink(const Poly &p) {
    Polynomial_Link pl;
    for (auto &kv : p.coef) {
        int deg = kv.first;
        double coef = kv.second;
        if (fabs(coef) < 1e-12) continue;
        pl.insert(term(coef, deg));
    }
    return pl;
}

void test_numeric_expressions() {
    cout << "Running numeric expression tests...\n";
    auto ev = [](const string &s)->double {
        return ExpressionEvaluator::evaluate(s, {});
    };

    assert(fabs(ev("3+4*2") - 11.0) < EPS);
    assert(fabs(ev("3+4*2.5") - 13.0) < EPS);
    assert(fabs(ev("(1+2)^3 / 9") - 3.0) < EPS);
    assert(fabs(ev("-3+5") - 2.0) < EPS);
    assert(fabs(ev("7/2") - 3.5) < EPS);
    // unary minus with no space: 3--2 => 5
    assert(fabs(ev("3--2") - 5.0) < EPS);
    // power right-associative: 2^3^2 = 2^(3^2) = 2^9 = 512
    assert(fabs(ev("2^3^2") - 512.0) < EPS);

    cout << "[OK] Numeric expression evaluation\n";
}

void test_polynomial_parsing_basic() {
    cout << "Running polynomial parsing tests...\n";

    // parse simple polynomial
    {
        string expr = "2.5*x^3 + 4*x - 1";
        Poly p = parseExpressionToPoly(expr, "x");
        auto it3 = p.coef.find(3);
        auto it1 = p.coef.find(1);
        auto it0 = p.coef.find(0);
        assert(it3 != p.coef.end() && fabs(it3->second - 2.5) < EPS);
        assert(it1 != p.coef.end() && fabs(it1->second - 4.0) < EPS);
        assert(it0 != p.coef.end() && fabs(it0->second + 1.0) < EPS);
    }

    // parse with parentheses and expansion
    {
        string expr = "(x+1)^2 - x";
        Poly p = parseExpressionToPoly(expr, "x");
        // (x+1)^2 - x = x^2 + x + 1
        assert(p.coef.size() >= 3);
        assert(p.coef.at(2) == 1.0);
        assert(p.coef.at(1) == 1.0);
        assert(p.coef.at(0) == 1.0);
    }

    cout << "[OK] Polynomial parsing basic\n";
}

void test_parse_errors() {
    cout << "Running parse-error tests...\n";
    // non-integer exponent should throw
    bool caught = false;
    try {
        parseExpressionToPoly("x^1.5", "x");
    } catch (const exception &e) {
        caught = true;
    }
    assert(caught);

    // division is not supported in polynomial parsing
    caught = false;
    try {
        parseExpressionToPoly("1/x", "x");
    } catch (const exception &e) {
        caught = true;
    }
    assert(caught);

    cout << "[OK] Polynomial parse error cases\n";
}

void test_poly_conversion_and_evaluation() {
    cout << "Running conversion and evaluation tests...\n";
    string expr = "2.5*x^3 + 4*x - 1";

    // parse to symbolic Poly
    Poly p = parseExpressionToPoly(expr, "x");

    // evaluate numeric with ExpressionEvaluator (variable map)
    double xval = 1.7;
    double want = ExpressionEvaluator::evaluate(expr, VarMap{{string("x"), xval}});

    // convert to Polynomial_Seq and evaluate
    Polynomial_Seq ps = polyToSeq(p);
    double got_seq = evalPolySeq(ps, xval);
    assert(fabs(got_seq - want) < 1e-7);

    // convert to Polynomial_Link and evaluate
    Polynomial_Link pl = polyToLink(p);
    double got_link = evalPolyLink(pl, xval);
    assert(fabs(got_link - want) < 1e-7);

    cout << "[OK] Conversion Poly -> Polynomial_* and numeric evaluation match\n";
}

int main() {
    cout << "=== Test suite: expressions & polynomial input ===\n";
    test_numeric_expressions();
    test_polynomial_parsing_basic();
    test_parse_errors();
    test_poly_conversion_and_evaluation();
    cout << "All expression & polynomial tests passed.\n";
    return 0;
}