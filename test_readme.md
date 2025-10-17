```markdown
# 测试说明 — expressions & polynomial input

此测试专注于：
- 四则运算表达式的数值求值（ExpressionEvaluator::evaluate）
- 将表达式解析为多项式（parseExpressionToPoly / Poly）
- 将解析得到的 Poly 转为 Polynomial_Seq / Polynomial_Link 并验证数值结果一致性
- 错误情形：非整数幂、在多项式解析中使用除法等应被拒绝

文件
- test_expr_poly.cpp — 单元测试（使用 assert）

编译
在项目根目录运行（确保所有头/源均在当前目录）：
```bash
rm -f test_expr_poly
g++ -std=c++17 -Wall -Wextra -O2 test_expr_poly.cpp *.cpp -o test_expr_poly
```
说明：第二个 `*.cpp` 用于确保链接到项目中其它实现（SeqList/LinkList 等）。如果你的项目中已有其它编译命令，请保证在链接时包含所有必要的 .o/.cpp 文件。

运行
```bash
./test_expr_poly
```

预期输出（示例）：
```
=== Test suite: expressions & polynomial input ===
Running numeric expression tests...
[OK] Numeric expression evaluation
Running polynomial parsing tests...
[OK] Polynomial parsing basic
Running parse-error tests...
[OK] Polynomial parse error cases
Running conversion and evaluation tests...
[OK] Conversion Poly -> Polynomial_* and numeric evaluation match
All expression & polynomial tests passed.
```

注意
- 如果运行时出现未定义符号或链接错误，请用全量编译（包含项目所有 .cpp），或检查头文件是否与实现匹配（例如是否有多个 evaluator 实现冲突）。
- 若出现断言失败，请复制终端的完整输出并贴回，我会基于失败的断言位置给出进一步排查建议。

```