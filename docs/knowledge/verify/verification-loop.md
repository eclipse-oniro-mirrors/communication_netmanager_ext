# netmanager_ext 验证循环与 Done 定义（verification-loop）

> 组件 `@ohos.netmanager_ext` | subsystem `communication` | 编译命令与测试实名见 [build-and-test.md](build-and-test.md)
> 本文定义"改完之后怎么算完"：六步循环 + Done 六条 + 回退规则。**没有走完本循环的改动不算完成。**

## 1. 六步验证循环

```
①定位任务类型 ──► ②读对应约束 ──► ③最小编译（含开关两态）──► ④跑改动矩阵测试
                                                                        │
⑥Done 核对 ◄───────── ⑤专项自测（并发 / crash 恢复 / 授权，按需）◄────┘
```

1. **定位**：按 [../routing/task-routing.md](../routing/task-routing.md) §1 确定任务域与先读文档；
2. **读约束**：expert 四篇按需精读——并发改动必读 [../expert/state-machine-concurrency.md](../expert/state-machine-concurrency.md)，IPC 新增必读 [../expert/permission-security-constraints.md](../expert/permission-security-constraints.md)，跨仓触碰必读 [../expert/cross-component-boundaries.md](../expert/cross-component-boundaries.md)，同形历史案例查 [../expert/historical-failures.md](../expert/historical-failures.md)；
3. **最小编译**：只编改动域组（见 [build-and-test.md](build-and-test.md) §1）；若改动依赖 `netmanager_ext_config.gni` 开关或外部硬件开关，**true/false 两态各编一次**（§4）；
4. **矩阵测试**：按 [build-and-test.md](build-and-test.md) §3 改动→必跑表执行，禁止只跑全绿不跑关联；
5. **专项自测**（命中才做，做完必须留记录）：
   - 并发类：开关压测（共享连开连关、回调并发注册/注销）、锁序场景反复执行不得卡死；
   - crash 恢复类：kill service 进程后验证热点不误重开（5842ca2）、子进程无残留、client 事件仍在（124a016/4f6fff0 三查）；
   - 授权/权限类：非授权调用方全路径拒绝；EDM 禁用策略生效；
   - 解析类：畸形报文 fuzz 喂入无泄漏无越界（51e8b7d 样板：修复必须带可复现用例）；
6. **Done 核对**：逐条过 §3。

## 2. 停止条件（出现即止步，禁止带病合入）

- 任一矩阵测试失败且无修复方案；
- 开关两态任一态编译失败；
- 触碰红区大文件（[build-and-test.md](build-and-test.md) §5）但未绑定对应回归案例复验。

## 3. Done 定义（六条全满足才算完成）

1. **编译成功**：改动域组编译通过；涉开关改动两态均通过；
2. **矩阵测试全过**：[build-and-test.md](build-and-test.md) §3 对应行全部通过；
3. **红区绑定回归**：红区/历史案例区改动已复跑对应案例验证要点（[../expert/historical-failures.md](../expert/historical-failures.md) §2）；
4. **API 兼容未破坏**：`interfaces/kits/js/*.d.ts` 基线与 `interfaces/innerkits` 交付 SO 头文件无未评审变更；多语言绑定成套同步（JS/C/ANI/native 客户端）；
5. **红线零违反**：未自造权限字符串/错误码；未直连内核（一律经 base `NetsysController`）；未删测试求绿；回调全判空且注册/注销成对；
6. **格式检查**：`clang-format` 校验通过。

## 4. 回退方案

- 验证不过且短期无法修复：`git revert` 对应提交，不留带病代码进主干；
- 回退 1154/1155 SA profile、权限常量引用、内核调用封装等**跨仓面**时按逆序回退（ext → base → 硬件仓）；
- **回退后必须回写**教训到 [../expert/historical-failures.md](../expert/historical-failures.md)（hash + 现象 + 根因 + 教训，只增不删，参照 netstack 25bd2e61 案例格式）。

## 5. 提交前检查清单（逐项打勾）

- [ ] 三项编辑前声明已做（任务类别 / 已读文档 / 适用约束，见 [AGENTS.md](../../../AGENTS.md) §5）；
- [ ] 编译命令（含两态）已跑且零错误；
- [ ] §3 矩阵测试全部通过，日志留档；
- [ ] 专项自测（并发/crash/授权/解析）按需完成；
- [ ] Done 六条逐条核对无缺；
- [ ] 新增测试 target、术语、SA 变更已回写对应知识文档（[build-and-test.md](build-and-test.md) / [../routing/vocabulary-routing.md](../routing/vocabulary-routing.md)）。

## 关联知识

- 测试实名与编译命令：[build-and-test.md](build-and-test.md)
- 失效案例与红区清单：[../expert/historical-failures.md](../expert/historical-failures.md)

## 适用场景

- 任何代码改动（功能/修复/重构）完成后的收尾验证；
- MR 评审者按 §5 清单核对贡献者自检；
- 判断一次回退是否合规、教训是否已回写。
