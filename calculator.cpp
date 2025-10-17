
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

int main() {
    SeqList<int> seqList;
    LinkList<int> linkList;

    // 存储命名的多项式集合
    unordered_map<string, Polynomial_Seq> polySeqMap;
    unordered_map<string, Polynomial_Link> polyLinkMap;

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