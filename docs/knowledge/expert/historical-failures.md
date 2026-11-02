# netmanager_ext 历史高危修复台账（historical-failures）

> 组件 `@ohos.netmanager_ext` | subsystem `communication` | 崩溃/竞态任务先读本文，再看 [state-machine-concurrency.md](state-machine-concurrency.md)
> 每个条目＝真实 git hash + 现象/根因/修复落点/验证要点。**同区域再改动时必须复跑对应验证，禁止只改代码不复验。**

## 1. 失效模式 TOP 排行（ext 特化）

| 排名 | 失效模式 | 历史案例 | 热点文件 |
|---|---|---|---|
| TOP1 | 并发/竞态/UAF/ABBA 死锁 | 2dd7ac1、30cd0bb、4e24608 | `networkshare_tracker.cpp`、`ethernet_management.cpp`、`networkvpn_service.cpp` |
| TOP2 | 空指针/判空缺失 | f5cc878、ec54b35 | `router_advertisement_daemon.cpp`、`networkshare_upstreammonitor.cpp` |
| TOP3 | 内存泄漏（异常分支/报文解析） | 51e8b7d、23c57cd | `mdns_packet_parser.cpp`、ethernetclient/mdnsclient Parcel 4 文件 |
| TOP4 | SA/进程崩溃恢复状态错误 | 5842ca2、4f6fff0、124a016 | `networkshare_service.cpp`、`networkvpn_service.cpp`、`networkvpn_client.cpp` |
| TOP5 | 调用方身份校验顺序 | VPN EDM 区（`networkvpn_service.cpp:1068-1069`） | `networkvpn_service.cpp` |

## 2. 台账（按 hash）

### 2dd7ac1 — tracker 回调集合 UAF → atomic 位图
- ❌ 现象：共享客户端注册/注销并发时攻击 `clientRequestsVector_` 容器，UAF 崩溃。
- 根因：容器非线程安全，多回调线程并发读写。
- ✅ 修复落点：`networkshare_tracker.cpp`，容器改 `clientRequestsBitMask_` 位图 + atomic 访问。
- 验证要点：并发注册/注销回调压测必须通过；后续动 tracker 任何成员先想线程域。

### f5cc878 — RA 守护 mac 空指针
- ❌ 现象：网络共享偶现 crash，mac 地址空指针。
- 根因：RA 守护在 mac 未就绪窗口即被使用。
- ✅ 修复落点：`services/networksharemanager/include/router_advertisement_daemon.h` + `src/router_advertisement_daemon.cpp`（判空后再用）。
- 验证要点：共享开启瞬间读写 mac 的时序用例。

### 30cd0bb — 以太网配置 ABBA 死锁
- ❌ 现象：配置线程互锁，`ethernet_management.cpp` 两把锁交叉获取。
- 根因：锁序不一致（A→B 与 B→A 并存）。
- ✅ 修复落点：`services/ethernetmanager/src/ethernet_management.cpp`（1131 行红区）统一锁序。
- 验证要点：并发 set 配置（IP 模式/使能）压测不得卡死。

### 4e24608 — `setVpnPidMap_` 并发写竞态
- ❌ 现象：VPN 开关并发时 pid 映射表写坏。
- ✅ 修复落点：`networkvpn_service.cpp` 加锁/原子化。
- 验证要点：VPN 连续开关与多调用方并发用例。

### 51e8b7d — mDNS UDP �畸形报文注入内存泄漏
- ❌ 现象：构造畸形 UDP 报文可注入导致泄漏。
- 根因：解析器未先校验长度即拷贝。
- ✅ 修复落点：`services/mdnsmanager/src/protocol/mdns_packet_parser.cpp`（先校验后拷贝），**并配套新增 unittest**（`mdns_protocol_test`）。
- 验证要点：`mdns_protocol_test` + 畸形报文 fuzz 通过；同类解析改动照抄"先校验后拷贝"。

### 4f6fff0 — VPN 崩溃后子进程未清理
- ❌ 现象：service 崩溃后遗留 VPN 守护子进程。
- ✅ 修复落点：`networkvpn_service.cpp` 子进程清理路径。
- 验证要点：kill service 后验证无残留子进程、再次建链正常。

### 5842ca2 — crash 恢复后已关闭热点被重新打开
- ❌ 现象：进程 crash 恢复后，无连接（原本自动关闭）的热点又被打开。
- 根因：恢复路径按残留状态重建共享，未判断自动关闭语义。
- ✅ 修复落点：`networkshare_service.cpp` + `networkshare_tracker.cpp` 恢复分支。
- 验证要点：crash 恢复后共享状态与预期一致；与 TOP4 组合看 [state-machine-concurrency.md](state-machine-concurrency.md)。

### ec54b35 — upstream 回调空指针
- ❌ 现象：上行回调崩在 null 检查缺失。
- ✅ 修复落点：`networkshare_upstreammonitor.cpp` 共 4 处判空（:115/:178/:187/:196）。
- 验证要点：上游网络快速切换/断开时共享不崩。

### 23c57cd — IPC 序列化异常分支内存泄漏
- ❌ 现象：Parcel 异常分支泄漏。
- ✅ 修复落点：`frameworks/native/ethernetclient/src/{configuration_parcel_ipc,interface_configuration,mac_address_info}.cpp`、`frameworks/native/mdnsclient/src/mdns_service_info.cpp`。
- 验证要点：异常报文/坏 Parcel 输入下无泄漏；写 `Marshalling`/`Unmarshalling` 时每个提前 return 分支都要释放。

### 124a016 — VPN client SA 重启后失联
- ❌ 现象：service SA 重启后 client 不再收事件。
- 根因：client 未按 `OnAddSystemAbility` 重新订阅。
- ✅ 修复落点：`frameworks/native/netvpnclient/src/networkvpn_client.cpp`（`SubscribeSystemAbility:153`/`OnAddSystemAbility:167`）。
- 验证要点：重启 SA 后回调仍然可达。

## 3. 沉淀规则（红线）

- ⛔ 禁止删除/注释失败测试让构建变绿；新修复**必须**带上可复现用例（51e8b7d 是样板）。
- ⛔ 红区文件改动（`networkvpn_service.cpp` 2960 行、`networkshare_tracker.cpp` 1755 行、`ethernet_management.cpp` 1131 行）必须绑定 §2 对应验证再合入。
- ⚠️ 回退某修复前先回读本表对应条目，确认不会复现原始缺陷；回退后必须把失效模式回写本表（参照 netstack 25bd2e61 案例格式：hash + 现象 + 根因 + 教训，**只增不删**）。
- ⚠️ 崩溃恢复类改动（TOP4）必须在 crash 注入场景下验证，禁止只跑正常路径。

## 关联知识

- 并发模式细则：[state-machine-concurrency.md](state-machine-concurrency.md)
- 验证命令与测试实名：[../verify/build-and-test.md](../verify/build-and-test.md)
- 现象快查入口：[../routing/task-routing.md](../routing/task-routing.md) §2

## 适用场景

- 排查崩溃/竞态/泄漏前，先查本表是否已有同形案例；
- 评审红区文件 MR 时按 §1 排行核对风险；
- 回退或重构历史修复区时，用作回归清单。
