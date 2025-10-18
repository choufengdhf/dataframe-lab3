
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <unordered_map>
#include <cmath>

#include "SeqList.hpp"
#include "LinkList.hpp"
#include "ui.hpp"
#include "evaluator.hpp"

using namespace std;
using namespace expr_eval;

// 辅助：从 map 中以索引方式选择一个名字（返回空字符串表示取消/无选择）
template<typename MapT>
static string chooseNameFromMap(const MapT& mp) {
    if (mp.empty()) {
        cout << "当前没有已创建的多项式。\n";
        return "";
    }
    vector<string> names;
    names.reserve(mp.size());
    int idx = 1;
    cout << "已创建的多项式：\n";
    for (const auto &kv : mp) {
        cout << "  " << idx << ". " << kv.first << "\n";
        names.push_back(kv.first);
        ++idx;
    }
    cout << "输入编号选择（0 取消）： ";
    int sel;
    while (!(cin >> sel)) {
        cout << "无效输入，请输入编号： ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (sel <= 0 || sel > (int)names.size()) {
        cout << "已取消选择或编号不合法。\n";
        return "";
    }
    return names[sel - 1];
}

// 评估顺序表形式的多项式在 x 处的值
static double evaluatePolynomial(const Polynomial_Seq& p, double x) {
    double res = 0.0;
    for (term* it = p.begin(); it != p.end(); ++it) {
        res += static_cast<double>(it->coefficient) * std::pow(x, it->degree);
    }
    return res;
}

// 评估链表形式的多项式在 x 处的值
static double evaluatePolynomial(const Polynomial_Link& p, double x) {
    double res = 0.0;
    // 使用范围循环（ordered_LinkList 提供迭代器）
    for (auto t : p) {
        res += static_cast<double>(t.coefficient) * std::pow(x, t.degree);
    }
    return res;
}

// 保存多项式结果：询问是否保存并处理命名冲突
template<typename PolyT, typename MapT>
static void maybeSaveResult(const PolyT& result, MapT& mp) {
    cout << "是否保存结果为新的命名多项式？(y/n)：";
    string ans; getline(cin, ans);
    if (!ans.empty() && (ans[0]=='y' || ans[0]=='Y')) {
        cout << "请输入新多项式名称：";
        string nm; getline(cin, nm);
        if (nm.empty()) { cout << "名称为空，取消保存。\n"; return; }
        if (mp.find(nm) != mp.end()) {
            cout << "该名称已存在，是否覆盖？(y/n)：";
            string qq; getline(cin, qq);
            if (qq.empty() || !(qq[0]=='y' || qq[0]=='Y')) {
                cout << "取消保存。\n"; return;
            }
        }
        mp[nm] = result;
        cout << "已保存为 '" << nm << "'。\n";
    }
}
// ------------------ 用户自定义函数（DEF / RUN 支持） ------------------
struct UserFunction {
    std::string param; // 形参名
    std::string body;  // 函数体表达式（字符串形式）
};

static inline std::string trimCopy(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && isspace((unsigned char)s[a])) ++a;
    while (b > a && isspace((unsigned char)s[b-1])) --b;
    return s.substr(a, b-a);
}

static bool isIdentChar(char c) {
    return std::isalnum((unsigned char)c) || c == '_';
}

// 判断一个 token 是否为纯数字（可带小数点和符号）
static bool isNumberToken(const std::string &t) {
    if (t.empty()) return false;
    size_t i = 0;
    if (t[0] == '+' || t[0] == '-') ++i;
    bool hasDigit = false, hasDot = false;
    for (; i < t.size(); ++i) {
        if (std::isdigit((unsigned char)t[i])) hasDigit = true;
        else if (t[i] == '.' && !hasDot) hasDot = true;
        else return false;
    }
    return hasDigit;
}

// 将 body 中所有独立出现的形参替换为 argText（argText 应该已经根据需要用括号包裹）
static std::string replaceParamIdentifier(const std::string &body, const std::string &param, const std::string &argText) {
    if (param.empty()) return body;
    // 把 body 和 argText 都 tokenize，然后在 token 级别替换 param -> argTokens
    vector<string> bodyToks = ExpressionEvaluator::tokenize(body);
    vector<string> argToks = ExpressionEvaluator::tokenize(argText);

    std::string out;
    out.reserve(body.size() + argText.size() * 2);

    for (size_t i = 0; i < bodyToks.size(); ++i) {
        const string &tk = bodyToks[i];
        if (tk == param) {
            // 插入 argTokens（不加额外括号——调用方应决定是否包裹）
            for (const auto &at : argToks) out += at;
        } else {
            out += tk;
        }
    }
    return out;
}

// 递归展开表达式中的用户函数调用，depth 用于防止无限递归
static std::string expandFunctionsRecursive(const std::string &expr, const std::unordered_map<std::string, UserFunction> &userFuncs, int depth = 0) {
    if (depth > 30) throw std::runtime_error("函数展开深度过大（可能存在递归）");
    std::string s = expr;
    size_t i = 0;
    while (i < s.size()) {
        // 寻找形如 name(...)
        if (std::isalpha((unsigned char)s[i]) || s[i]=='_') {
            size_t idStart = i;
            size_t j = i + 1;
            while (j < s.size() && isIdentChar(s[j])) ++j;
            std::string name = s.substr(idStart, j - idStart);
            // 跳过空白后看是否是 '('
            size_t k = j;
            while (k < s.size() && isspace((unsigned char)s[k])) ++k;
            if (k < s.size() && s[k] == '(') {
                // 解析括号内的内容，支持嵌套
                size_t p = k + 1;
                int depthParen = 1;
                while (p < s.size() && depthParen > 0) {
                    if (s[p] == '(') ++depthParen;
                    else if (s[p] == ')') --depthParen;
                    ++p;
                }
                if (depthParen != 0) {
                    // 括号不匹配，跳过
                    i = j;
                    continue;
                }
                std::string argText = s.substr(k + 1, p - (k + 1));
                // 先递归展开参数内部的函数调用
                std::string expandedArg = expandFunctionsRecursive(argText, userFuncs, depth + 1);
                // 查找是否有定义的函数
                auto it = userFuncs.find(name);
                if (it != userFuncs.end()) {
                    // 用括号包裹参数以保留优先级
                    std::string wrappedArg = "(" + expandedArg + ")";
                    std::string replaced = replaceParamIdentifier(it->second.body, it->second.param, wrappedArg);
                    // 递归展开替换后可能产生的新函数调用
                    std::string finalExpanded = expandFunctionsRecursive(replaced, userFuncs, depth + 1);
                    // replace s[idStart .. p-1] with finalExpanded
                    s.replace(idStart, p - idStart, finalExpanded);
                    // 继续从 idStart 位置处理（新的内容可能包含更多调用）
                    i = idStart + finalExpanded.size();
                    continue;
                } else {
                    // 未定义的函数名，跳过但保留对参数的展开结果
                    s.replace(k + 1, p - (k + 1), expandedArg);
                    i = p;
                    continue;
                }
            } else {
                // 不是函数调用，继续
                i = j;
                continue;
            }
        } else {
            ++i;
        }
    }
    return s;
}

// 处理 DEF 语句：格式 DEF name(param)=expr
static bool handleDEF(const std::string &line, std::unordered_map<std::string, UserFunction> &userFuncs, std::string &errMsg) {
    // 简单解析
    size_t p = 0;
    while (p < line.size() && isspace((unsigned char)line[p])) ++p;
    // find first space
    size_t sp = line.find_first_of(" \t", p);
    size_t start = (sp==std::string::npos) ? p : sp+1;
    std::string rest = trimCopy(line.substr(start));
    // 找到 '='
    size_t eq = rest.find('=');
    if (eq == std::string::npos) { errMsg = "缺少 '='"; return false; }
    std::string left = trimCopy(rest.substr(0, eq));
    std::string right = trimCopy(rest.substr(eq + 1));
    // left 应为 name(param)
    size_t lp = left.find('(');
    size_t rp = left.rfind(')');
    if (lp == std::string::npos || rp == std::string::npos || rp <= lp) { errMsg = "左侧应为 name(param) 格式"; return false; }
    std::string name = trimCopy(left.substr(0, lp));
    std::string param = trimCopy(left.substr(lp + 1, rp - lp - 1));
    if (name.empty() || param.empty()) { errMsg = "函数名或参数为空"; return false; }
    // 基本合法性检查：name 和 param 都应为标识符
    if (!std::isalpha((unsigned char)name[0])) { errMsg = "函数名不是合法标识符"; return false; }
    if (!std::isalpha((unsigned char)param[0])) { errMsg = "参数名不是合法标识符"; return false; }
    for (char c : name) if (!isIdentChar(c)) { errMsg = "函数名不是合法标识符"; return false; }
    for (char c : param) if (!isIdentChar(c)) { errMsg = "参数名不是合法标识符"; return false; }
    // 在存储前展开 RHS 中已定义的函数调用，并尝试将其化简为多项式形式
    try {
        std::string expandedRight = expandFunctionsRecursive(right, userFuncs);

        // 新增：规范化：去除形如 "(param)" 的多余括号（多次迭代以处理嵌套情况）
        // 例如把 "1+(x)" -> "1+x"，把 "((x))" -> "x"
        if (!param.empty()) {
            std::string patternL = "(" + param + ")";
            // 反复替换直到没有匹配（避免嵌套残留）
            size_t pos;
            while ((pos = expandedRight.find("(" + param + ")")) != std::string::npos) {
                expandedRight.erase(pos, param.size() + 2);
                expandedRight.insert(pos, param);
            }
            // 另外尝试去掉多余的双括号，如 "((x))" -> "(x)" 再由上面去掉
            // 已上循环可以多次运行，通常足够
        }

        // 尝试将展开后的表达式解析为多项式
        try {
            Poly p = parseExpressionToPoly(expandedRight, param);
            // 将 Poly 转回标准表达式字符串（按降幂遍历）
            std::ostringstream oss;
            bool firstTerm = true;
            for (auto &kv : p.coef) {
                int deg = kv.first;
                double coef = kv.second;
                if (std::fabs(coef) < 1e-12) continue;
                if (!firstTerm) oss << (coef < 0 ? '-' : '+');
                else if (coef < 0) oss << '-';
                double absC = std::fabs(coef);
                auto fmtNum = [&](double v)->std::string{
                    long long iv = llround(v);
                    if (std::fabs(v - (double)iv) < 1e-12) return std::to_string(iv);
                    std::ostringstream ss; ss.setf(std::ios::fixed); ss.precision(6); ss << v; std::string s = ss.str();
                    if (s.find('.') != std::string::npos) {
                        while (!s.empty() && s.back() == '0') s.pop_back();
                        if (!s.empty() && s.back() == '.') s.pop_back();
                    }
                    return s;
                };
                if (deg == 0) {
                    oss << fmtNum(absC);
                } else {
                    if (std::fabs(absC - 1.0) >= 1e-12) oss << fmtNum(absC) << '*';
                    oss << param;
                    if (deg != 1) oss << '^' << deg;
                }
                firstTerm = false;
            }
            std::string stored = oss.str();
            if (stored.empty()) stored = "0";
            UserFunction uf; uf.param = param; uf.body = stored;
            userFuncs[name] = uf;
            return true;
        } catch (const std::exception &e) {
            // 不能解析为多项式，保存展开后的表达式字符串（已展开内部函数），
            // 但先再做一次简单的括号清理，将 "(x)" -> "x" 等
            std::string cleaned = trimCopy(expandedRight);
            if (!param.empty()) {
                size_t pos;
                while ((pos = cleaned.find("(" + param + ")")) != std::string::npos) {
                    cleaned.erase(pos, param.size() + 2);
                    cleaned.insert(pos, param);
                }
            }
            UserFunction uf; uf.param = param; uf.body = cleaned;
            userFuncs[name] = uf;
            return true;
        }
    } catch (const std::exception &e) {
        errMsg = std::string("在展开 RHS 时出错: ") + e.what();
        return false;
    }
}   

// 处理 RUN 语句：支持 RUN f(y) （展示），RUN f(5)（计算），RUN expr（计算）
static bool handleRUN(const std::string &line, const std::unordered_map<std::string, UserFunction> &userFuncs, std::string &outStr, bool &isPrinted) {
    // 去掉前缀 RUN
    size_t p = 0; while (p < line.size() && isspace((unsigned char)line[p])) ++p;
    size_t sp = line.find_first_of(" \t", p);
    size_t start = (sp==std::string::npos) ? p : sp+1;
    std::string rest = trimCopy(line.substr(start));
    if (rest.empty()) { outStr = "RUN 后缺少表达式"; return false; }
    // 判断是否为 name(arg)
    size_t lp = rest.find('(');
    size_t rp = rest.rfind(')');
    if (lp != std::string::npos && rp != std::string::npos && rp > lp) {
        std::string name = trimCopy(rest.substr(0, lp));
        std::string arg = trimCopy(rest.substr(lp + 1, rp - lp - 1));
        auto it = userFuncs.find(name);
        if (it != userFuncs.end()) {
            // 若参数为标识符（字母开头且仅含标识符字符） -> 展示替换后的表达式
            bool argIsIdent = !arg.empty() && std::isalpha((unsigned char)arg[0]);
            for (char c : arg) if (!isIdentChar(c)) { argIsIdent = false; break; }
            if (argIsIdent) {
                std::string displayed = replaceParamIdentifier(it->second.body, it->second.param, arg);
                outStr = displayed;
                isPrinted = true;
                return true;
            }
            // 否则视为表达式 / 数字：先展开内部函数，再用表达式求值
            try {
                std::string expandedArg = expandFunctionsRecursive(arg, userFuncs);
                std::string wrappedArg = "(" + expandedArg + ")";
                std::string replaced = replaceParamIdentifier(it->second.body, it->second.param, wrappedArg);
                std::string finalExpanded = expandFunctionsRecursive(replaced, userFuncs);
                double val = ExpressionEvaluator::evaluate(finalExpanded, VarMap{});
                outStr = std::to_string(val);
                isPrinted = false;
                return true;
            } catch (const std::exception &e) {
                outStr = std::string("求值错误: ") + e.what();
                return false;
            }
        }
    }
    // 不是 simple name(arg) 或 name 未定义 -> 作为通用表达式处理（先展开已知函数调用再求值）
    try {
        std::string expanded = expandFunctionsRecursive(rest, userFuncs);
        double val = ExpressionEvaluator::evaluate(expanded, VarMap{});
        outStr = std::to_string(val);
        isPrinted = false;
        return true;
    } catch (const std::exception &e) {
        outStr = std::string("求值错误: ") + e.what();
        return false;
    }
}

// ------------------ end DEF/RUN ------------------

int main() {
    SeqList<int> seqList;
    LinkList<int> linkList;

    // 存储命名的多项式集合
    unordered_map<string, Polynomial_Seq> polySeqMap;
    unordered_map<string, Polynomial_Link> polyLinkMap;
    // 存储用户自定义函数（DEF / RUN）
    std::unordered_map<std::string, UserFunction> userFuncs;

    vect_Seq vect_seq1, vect_seq2;
    vect_Link vect_link1, vect_link2;

    bool running = true;
    while (running) {
        printMainMenu();
        int moduleChoice;
        if (!(cin >> moduleChoice)) {
            cout << "无效输入，请输入数字选项。\n";
            clearCin();
            continue;
        }
        clearCin();

        switch (moduleChoice) {
        case 1: { // 一元多项式（顺序表）
            bool back = false;
            while (!back) {
                printSeqPolyMenu();
                int op; cin >> op; clearCin();
                switch (op) {
                    case 1: { // 创建并命名多项式
                        cout << "请输入多项式名称：";
                        string name; getline(cin, name);
                        if (name.empty()) { cout << "名称不能为空。\n"; break; }
                        if (polySeqMap.find(name) != polySeqMap.end()) {
                            cout << "该名称已存在，是否覆盖？(y/n)：";
                            string ans; getline(cin, ans);
                            if (ans.empty() || !(ans[0]=='y' || ans[0]=='Y')) {
                                cout << "取消创建。\n"; break;
                            }
                        }
                        cout << "请输入多项式内容：\n";
                        Polynomial_Seq p = inputPolynomial_Seq();
                        polySeqMap[name] = p;
                        cout << "已创建多项式 '" << name << "'。\n";
                        break;
                    }
                    case 2: { // 列出多项式
                        if (polySeqMap.empty()) { cout << "尚无已创建的多项式。\n"; break; }
                        cout << "已创建的顺序表多项式：\n";
                        for (auto &kv : polySeqMap) {
                            cout << "---- " << kv.first << " : ";
                            kv.second.print();
                        }
                        break;
                    }
                    case 3: { // 删除多项式
                        string sel = chooseNameFromMap(polySeqMap);
                        if (sel.empty()) break;
                        polySeqMap.erase(sel);
                        cout << "已删除 '" << sel << "'.\n";
                        break;
                    }
                    case 4: { // 加法（选择两个已创建多项式）
                        if (polySeqMap.size() < 2) { cout << "至少需要两个已创建的多项式进行加法。\n"; break; }
                        cout << "选择第一个加数：\n";
                        string a = chooseNameFromMap(polySeqMap);
                        if (a.empty()) break;
                        cout << "选择第二个加数：\n";
                        string b = chooseNameFromMap(polySeqMap);
                        if (b.empty()) break;
                        Polynomial_Seq res = polySeqMap[a] + polySeqMap[b];
                        cout << "加法结果：";
                        res.print();
                        maybeSaveResult<Polynomial_Seq>(res, polySeqMap);
                        break;
                    }
                    case 5: { // 减法
                        if (polySeqMap.size() < 2) { cout << "至少需要两个已创建的多项式进行减法。\n"; break; }
                        cout << "选择被减数：\n";
                        string a = chooseNameFromMap(polySeqMap);
                        if (a.empty()) break;
                        cout << "选择减数：\n";
                        string b = chooseNameFromMap(polySeqMap);
                        if (b.empty()) break;
                        Polynomial_Seq res = polySeqMap[a] - polySeqMap[b];
                        cout << "减法结果：";
                        res.print();
                        maybeSaveResult<Polynomial_Seq>(res, polySeqMap);
                        break;
                    }
                    case 6: { // 乘法
                        if (polySeqMap.size() < 2) { cout << "至少需要两个已创建的多项式进行乘法。\n"; break; }
                        cout << "选择第一个乘数：\n";
                        string a = chooseNameFromMap(polySeqMap);
                        if (a.empty()) break;
                        cout << "选择第二个乘数：\n";
                        string b = chooseNameFromMap(polySeqMap);
                        if (b.empty()) break;
                        Polynomial_Seq res = polySeqMap[a] * polySeqMap[b];
                        cout << "乘法结果：";
                        res.print();
                        maybeSaveResult<Polynomial_Seq>(res, polySeqMap);
                        break;
                    }
                    case 7: { // 求导
                        if (polySeqMap.empty()) { cout << "尚无已创建的多项式。\n"; break; }
                        cout << "选择要求导的多项式：\n";
                        string a = chooseNameFromMap(polySeqMap);
                        if (a.empty()) break;
                        cout << "输入求导阶数（非负整数）：";
                        int dnum; while (!(cin >> dnum)) { cout << "请输入整数："; cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); }
                        clearCin();
                        Polynomial_Seq res = polySeqMap[a]; // 拷贝
                        res.differentiate(dnum);
                        cout << "求导结果：";
                        res.print();
                        maybeSaveResult<Polynomial_Seq>(res, polySeqMap);
                        break;
                    }
                    case 8: { // 赋值（在某个 x 点求值）
                        if (polySeqMap.empty()) { cout << "尚无已创建的多项式。\n"; break; }
                        cout << "选择要赋值/求值的多项式：\n";
                        string a = chooseNameFromMap(polySeqMap);
                        if (a.empty()) break;
                        cout << "请输入 " << polySeqMap[a].getVariableName() << " 的值（例如 2 或 3.5）：";
                        double xv;
                        while (!(cin >> xv)) { cout << "无效数字，请重输："; cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); }
                        clearCin();
                        double val = evaluatePolynomial(polySeqMap[a], xv);
                        cout << "多项式 " << a << " 在 " << polySeqMap[a].getVariableName() << "=" << xv << " 处的值为: " << val << "\n";
                        break;
                    }
                    case 0:
                        back = true; break;
                    default:
                        cout << "无效选择，请重新输入！\n";
                }
            }
            break;
        }
        case 2: { // 一元多项式（链表）
            bool back = false;
            while (!back) {
                printLinkPolyMenu();
                int op; cin >> op; clearCin();
                switch (op) {
                    case 1: { // 创建并命名多项式（链表）
                        cout << "请输入多项式名称：";
                        string name; getline(cin, name);
                        if (name.empty()) { cout << "名称不能为空。\n"; break; }
                        if (polyLinkMap.find(name) != polyLinkMap.end()) {
                            cout << "该名称已存在，是否覆盖？(y/n)：";
                            string ans; getline(cin, ans);
                            if (ans.empty() || !(ans[0]=='y' || ans[0]=='Y')) {
                                cout << "取消创建。\n"; break;
                            }
                        }
                        cout << "请输入多项式内容（链表）：\n";
                        Polynomial_Link p = inputPolynomial_Link();
                        polyLinkMap[name] = p;
                        cout << "已创建多项式 '" << name << "'（链表）。\n";
                        break;
                    }
                    case 2: { // 列出多项式（链表）
                        if (polyLinkMap.empty()) { cout << "尚无已创建的多项式（链表）。\n"; break; }
                        cout << "已创建的链表多项式：\n";
                        for (auto &kv : polyLinkMap) {
                            cout << "---- " << kv.first << " : ";
                            kv.second.print();
                        }
                        break;
                    }
                    case 3: { // 删除
                        string sel = chooseNameFromMap(polyLinkMap);
                        if (sel.empty()) break;
                        polyLinkMap.erase(sel);
                        cout << "已删除 '" << sel << "'（链表）。\n";
                        break;
                    }
                    case 4: { // 加法
                        if (polyLinkMap.size() < 2) { cout << "至少需要两个已创建的多项式进行加法。\n"; break; }
                        cout << "选择第一个加数：\n";
                        string a = chooseNameFromMap(polyLinkMap);
                        if (a.empty()) break;
                        cout << "选择第二个加数：\n";
                        string b = chooseNameFromMap(polyLinkMap);
                        if (b.empty()) break;
                        Polynomial_Link res = polyLinkMap[a] + polyLinkMap[b];
                        cout << "加法结果：";
                        res.print();
                        maybeSaveResult<Polynomial_Link>(res, polyLinkMap);
                        break;
                    }
                    case 5: { // 减法
                        if (polyLinkMap.size() < 2) { cout << "至少需要两个已创建的多项式进行减法。\n"; break; }
                        cout << "选择被减数：\n";
                        string a = chooseNameFromMap(polyLinkMap);
                        if (a.empty()) break;
                        cout << "选择减数：\n";
                        string b = chooseNameFromMap(polyLinkMap);
                        if (b.empty()) break;
                        Polynomial_Link res = polyLinkMap[a] - polyLinkMap[b];
                        cout << "减法结果：";
                        res.print();
                        maybeSaveResult<Polynomial_Link>(res, polyLinkMap);
                        break;
                    }
                    case 6: { // 乘法
                        if (polyLinkMap.size() < 2) { cout << "至少需要两个已创建的多项式进行乘法。\n"; break; }
                        cout << "选择第一个乘数：\n";
                        string a = chooseNameFromMap(polyLinkMap);
                        if (a.empty()) break;
                        cout << "选择第二个乘数：\n";
                        string b = chooseNameFromMap(polyLinkMap);
                        if (b.empty()) break;
                        Polynomial_Link res = polyLinkMap[a] * polyLinkMap[b];
                        cout << "乘法结果：";
                        res.print();
                        maybeSaveResult<Polynomial_Link>(res, polyLinkMap);
                        break;
                    }
                    case 7: { // 求导
                        if (polyLinkMap.empty()) { cout << "尚无已创建的多项式（链表）。\n"; break; }
                        cout << "选择要求导的多项式：\n";
                        string a = chooseNameFromMap(polyLinkMap);
                        if (a.empty()) break;
                        cout << "输入求导阶数（非负整数）：";
                        int dnum; while (!(cin >> dnum)) { cout << "请输入整数："; cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); }
                        clearCin();
                        Polynomial_Link res = polyLinkMap[a]; // 拷贝
                        res.differentiate(dnum);
                        cout << "求导结果：";
                        res.print();
                        maybeSaveResult<Polynomial_Link>(res, polyLinkMap);
                        break;
                    }
                    case 8: { // 赋值（链表多项式在 x 点求值）
                        if (polyLinkMap.empty()) { cout << "尚无已创建的多项式（链表）。\n"; break; }
                        cout << "选择要赋值/求值的多项式：\n";
                        string a = chooseNameFromMap(polyLinkMap);
                        if (a.empty()) break;
                        cout << "请输入 " << polyLinkMap[a].getVariableName() << " 的值（例如 2 或 3.5）：";
                        double xv;
                        while (!(cin >> xv)) { cout << "无效数字，请重输："; cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); }
                        clearCin();
                        double val = evaluatePolynomial(polyLinkMap[a], xv);
                        cout << "链表多项式 " << a << " 在 " << polyLinkMap[a].getVariableName() << "=" << xv << " 处的值为: " << val << "\n";
                        break;
                    }
                    case 0:
                        back = true; break;
                    default:
                        cout << "无效选择，请重新输入！\n";
                }
            }
            break;
        }
        case 3: { // 顺序表（向量） - 保持原有功能
            bool back = false;
            while (!back) {
                printSeqVectMenu();
                int op; cin >> op; clearCin();
                switch (op) {
                    case 1: // 计算模长
                        vect_seq1 = inputVect_Seq();
                        cout << "模长：" << vect_seq1.module() << endl;
                        break;
                    case 2: // 点积
                        cout << "输入第一个向量（顺序表）：\n";
                        vect_seq1 = inputVect_Seq();
                        cout << "输入第二个向量（顺序表）：\n";
                        vect_seq2 = inputVect_Seq();
                        cout << "点积：" << (vect_seq1 * vect_seq2) << endl;
                        break;
                    case 3: // 夹角余弦
                        cout << "输入第一个向量（顺序表）：\n";
                        vect_seq1 = inputVect_Seq();
                        cout << "输入第二个向量（顺序表）：\n";
                        vect_seq2 = inputVect_Seq();
                        cout << "夹角余弦：" << cos(vect_seq1, vect_seq2) << endl;
                        break;
                    case 0:
                        back = true; break;
                    default:
                        cout << "无效选择，请重新输入！\n";
                }
            }
            break;
        }
        case 4: { // 链表（向量）
            bool back = false;
            while (!back) {
                printLinkVectMenu();
                int op; cin >> op; clearCin();
                switch (op) {
                    case 1: // 计算模长
                        vect_link1 = inputVect_Link();
                        cout << "模长：" << vect_link1.module() << endl;
                        break;
                    case 2: // 点积
                        cout << "输入第一个向量（链表）：\n";
                        vect_link1 = inputVect_Link();
                        cout << "输入第二个向量（链表）：\n";
                        vect_link2 = inputVect_Link();
                        cout << "点积：" << (vect_link1 * vect_link2) << endl;
                        break;
                    case 3: // 夹角余弦
                        cout << "输入第一个向量（链表）：\n";
                        vect_link1 = inputVect_Link();
                        cout << "输入第二个向量（链表）：\n";
                        vect_link2 = inputVect_Link();
                        cout << "夹角余弦：" << cos(vect_link1, vect_link2) << endl;
                        break;
                    case 0:
                        back = true; break;
                    default:
                        cout << "无效选择，请重新输入！\n";
                }
            }
            break;
        }
        case 5: { // 顺序表（基本线性表）
            bool back = false;
            while (!back) {
                printSeqListMenu();
                int op; cin >> op; clearCin();
                switch (op) {
                    case 1: createSeqList(seqList); break;
                    case 2:
                        cout << "请输入插入元素: ";
                        int val; cin >> val;
                        seqList.push_back(val);
                        cout << "插入成功！" << endl;
                        break;
                    case 3: traverseSeqList(seqList); break;
                    case 4: clearSeqList(seqList); break;
                    case 5: destroySeqList(seqList); break;
                    case 0: back = true; break;
                    default: cout << "无效选择，请重新输入！\n";
                }
            }
            break;
        }
        case 6: { // 链表（基本线性表）
            bool back = false;
            while (!back) {
                printLinkListMenu();
                int op; cin >> op; clearCin();
                switch (op) {
                    case 1: createLinkList(linkList); break;
                    case 2:
                        cout << "请输入插入元素: ";
                        int val; cin >> val;
                        linkList.tailinsert(val);
                        cout << "插入成功！" << endl;
                        break;
                    case 3: traverseLinkList(linkList); break;
                    case 4: clearLinkList(linkList); break;
                    case 5: destroyLinkList(linkList); break;
                    case 0: back = true; break;
                    default: cout << "无效选择，请重新输入！\n";
                }
            }
            break;
        }
        case 8: { // 编程界面：支持 DEF / RUN
            cout << "进入编程界面。输入 DEF <定义> 或 RUN <运行>，输入 EXIT 返回主菜单。\n";
            while (true) {
                cout << "prog> ";
                string line;
                if (!getline(cin, line)) break; // EOF
                string t = trimCopy(line);
                if (t.empty()) continue;
                // 取第一个 token
                size_t sp = t.find_first_of(" \t");
                string cmd = (sp==string::npos) ? t : t.substr(0, sp);
                if (cmd == "DEF" || cmd == "def") {
                    string err;
                    if (!handleDEF(t, userFuncs, err)) cout << "DEF 失败: " << err << "\n";
                    // 成功时静默返回
                    continue;
                } else if (cmd == "RUN" || cmd == "run") {
                    string out; bool isPrinted = false;
                    if (handleRUN(t, userFuncs, out, isPrinted)) {
                        cout << out << "\n";
                    } else {
                        cout << "RUN 失败: " << out << "\n";
                    }
                    continue;
                } else if (cmd == "EXIT" || cmd == "exit") {
                    break;
                } else {
                    // 当作表达式直接运行
                    string out; bool isPrinted = false;
                    if (handleRUN(string("RUN ") + t, userFuncs, out, isPrinted)) cout << out << "\n";
                    else cout << "运行失败: " << out << "\n";
                    continue;
                }
            }
            break;
        }
        case 7: { // 四则运算表达式求值（无变量）
            cout << "四则运算求值（不含变量）。输入一个表达式（例如 3+4*2 或 (1+2)^3/9），回车计算；\n";
            cout << "表达式: ";
            string expr;
            if (!getline(cin, expr) || expr.empty()) {
                // 用户输入空行 -> 返回主菜单
                break;
            }
            try {
                // 检测是否有标识符（变量），若有则拒绝（此模块为无变量）
                auto vars = ExpressionEvaluator::extractVariables(expr);
                if (!vars.empty()) {
                    cout << "表达式包含标识符（变量），当前模块只支持无变量表达式。检测到：";
                    for (auto &v : vars) cout << v << " ";
                    cout << "\n请使用不含变量的表达式，或在多项式模块中使用带变量的表达式。\n";
                    break;
                }
                double res = ExpressionEvaluator::evaluate(expr, VarMap{});
                cout << "结果: " << res << "\n";
            } catch (const exception &e) {
                cout << "求值异常: " << e.what() << "\n";
            }
            break;
        }
        case 0:
            running = false;
            cout << "感谢使用，已退出。\n"; break;
        default:
            cout << "无效选择，请重新输入！\n";
        }
    }
    return 0;
}
