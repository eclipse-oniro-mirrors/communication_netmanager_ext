# netmanager_ext 状态机与并发约束（state-machine-concurrency）

> 组件 `@ohos.netmanager_ext` | subsystem `communication` | 失效案例背景见 [historical-failures.md](historical-failures.md)
> ext 的崩溃大户不在协议栈而在**共享状态机的并发**与**SA 崩溃恢复**：本文是动这些区域前的强制读。

## 1. 并发域总图（谁在碰共享状态）

```
IPC Binder 线程（各 SA 请求）
事件回调线程（WiFi/蓝牙/USB 状态回调、DHCP 回调）
SA 恢复路径（OnAddSystemAbility / OnRemoveSystemAbility）
             │
             ▼
  NetworkShareService ─► NetworkShareTracker（请求方位图）
        │                       │
        ▼                       ▼
  主状态机（NAT/转发） ─► 子状态机（WIFI/USB/BT × 每下游口一实例）
        │                       │
        ▼                       ▼
  upstreammonitor ◄── 上游网络事件（netconn 回调经 base 代理）
```

⚠️ 任一竖列都可能在多线程同时进入；写成员前必须先回答"这条路径有几个并发入口"。

## 2. 主/子状态机模式（share 域核心）

- 两层职责：主状态机管 NAT/内核转发级开关（`EnableNat:280`/`:290`、`IpEnableForwarding:363`、`DisableNat:399`/`:401`）；子状态机按 `SharingIfaceType` 各自驱动（WIFI 复用 `wifi_hotspot.h:35`、热点模式 `:518`、DHCP `:954`/`:977`；USB=RNDIS；BT=PAN）。
- **对称规则**：`EnableNat`/`DisableNat`、`IpEnableForwarding`/关闭转发必须严格成对；只开不关＝泄漏残留内核规则（对照 `main_statemachine.cpp` 现有对称写法）。
- **幂等规则**：状态切换回调可能重入（热点快速开/关、USB 反复插拔），状态迁移必须幂等——同一事件连续到达两次不得二次起守护/二次改 NAT。
- `FAKE_DOWNSTREAM_IFACENAME`（`networkshare_main_statemachine.cpp:31`）：无真实下行口时的占位名，恢复逻辑依赖它判断"曾经的下游"；⚠️ 禁止把它当真实网卡名下发内核。
- crash 恢复（5842ca2）：恢复路径重建共享前必须核对"crash 前是否已自动关闭"，禁止盲按残留状态重开热点。

## 3. tracker 位图模式（TOP1 UAF 的答案）

- ❌ 教训（2dd7ac1）：容器 `clientRequestsVector_` 并发读写 → UAF。
- ✅ 现状：`networkshare_tracker.cpp` 用 `netshareRequester_`/`clientRequestsBitMask_` 位图 + atomic 访问。
- 规则：tracker 成员**只允许 atomic/锁保护访问**；新增请求方类型先扩位图位，禁止再引入无保护容器。

## 4. 锁纪律（ABBA 防线）

- ❌ 教训（30cd0bb）：`ethernet_management.cpp` 两把锁交叉获取 → 死锁。
- 规则：
  1. 单对象优先单锁；跨对象操作按**固定全局锁序**（建议：service 层锁 → management 锁 → 设备状态锁）。
  2. 锁内动作只做内存操作；⛔ 禁止锁内调 IPC/等待回调（易升级为分布式死锁）。
  3. 新增锁时必须在本文登记锁序，评审按 §1 并发入口核对。
- VPN 侧同形案例：`setVpnPidMap_` 竞态（4e24608，`networkvpn_service.cpp`）——map 类成员并发写必须持锁。

## 5. SA 崩溃恢复序（TOP4 类规范）

三个历史案例给出固定的恢复检查单：

| 环节 | 规范 | 案例 |
|---|---|---|
| service 恢复 | 重建状态前核对 crash 前语义（自动关闭的不得重开） | 5842ca2 |
| 子进程资源 | service 恢复入口先清残留守护子进程再接受新请求 | 4f6fff0 |
| client 侧重订阅 | `OnAddSystemAbility` 必须重新 `SubscribeSystemAbility` 恢复订阅链 | 124a016（`networkvpn_client.cpp:167`） |

- 规则：新增任何 SA 事件订阅，必须同时写 `OnAddSystemAbility` 恢复分支（样板：`networkvpn_client.cpp:153/:162/:167`；ethernet 侧 `ethernet_service.cpp:86` 等 SA 1151 就绪再 `InitManagement:102`）。
- ⚠️ 恢复路径测试必须在 crash 注入后验证状态Checksum，禁止只跑正常路径。

## 6. 回调生命周期规则

- ⚠️ 教训（ec54b35）：`networkshare_upstreammonitor.cpp` 4 处补判空（:115/:178/:187/:196）——**回调入口全部先判空再用**，上游对象可能在回调排队期间被销毁。
- IPC 回调注册/注销（`IVpnEventCallback` 等 IDL 回调）必须成对；注销后再回调＝UAF 前兆，参照 tracker 位图管理请求方。
- 多语言侧回调（JS/Native client）经 base 仓事件机制回 JS 线程，ext 不自建线程直调回调。

## 7. 自检清单（改状态机/共享成员前逐条打勾）

1. 找出该成员的全部并发入口（IPC/事件回调/恢复路径）？——对照 §1
2. 访问是否 atomic 或持锁？锁序是否登记于 §4？
3. 状态迁移是否幂等？开/关动作是否对称？
4. 是否触碰恢复路径？crash 注入用例是否已列？
5. 回调入口是否全部判空？注册/注销是否成对？
6. 红区文件（`networkshare_tracker.cpp` 1755 行、`networkvpn_service.cpp` 2960 行、`ethernet_management.cpp` 1131 行）改动是否已绑定 [../verify/build-and-test.md](../verify/build-and-test.md) 对应测试？

## 关联知识

- 案例台账：[historical-failures.md](historical-failures.md)
- 权限与身份校验顺序：[permission-security-constraints.md](permission-security-constraints.md)
- 验证循环：[../verify/verification-loop.md](../verify/verification-loop.md)

## 适用场景

- 改动 `networkshare_*`、`networkvpn_*`、`ethernet_management`、`mdns_protocol_impl` 中任何共享成员；
- 新增状态、新增事件回调、新增 SA 订阅；
- crash 恢复路径重构评审。
