# 以太网管理链路图（SA 1157）

> 本文描述以太网能力全链路：HDI 设备事件 → `EthernetService` → 管理器/DHCP → 向 base 注册供应商。改 `services/ethernetmanager`、`frameworks/native/ethernetclient`、`frameworks/js/napi/ethernet` 前必读。

## 1. 总链路

```
HDI 以太网驱动（libethernet_proxy_1.0， peripheral HDI）
   ↓ 设备插拔/链路事件
EthernetService（services/ethernetmanager/src/ethernet_service.cpp，SA 1157）
   ├─ OnAddSystemAbility(:86)：等待 SA 1151（net_conn）与 COMMON_EVENT_SERVICE_ID 就绪
   └─ InitManagement(:102) ↓
EthernetManagement（src/ethernet_management.cpp，1131 行红区）
   ↓ 管理口配置/DHCP/接口状态机
DevInterfaceState（src/dev_interface_state.cpp）
   ├─ RemoteRegisterNetSupplier(:161)   → NetManagerCenter → base 侧 NetConnService 注册供应商
   └─ RemoteUpdateNetLinkInfo(:188)     → 上报 linkInfo（地址/路由/DNS）
DHCP 客户端（dhcp_c_api.h，依赖仓 dhcp；controller 见 services 内调用）
   ↓ 获取 IPv4/IPv6
接口状态上报（interface_state_callback_{stub,proxy}，interfaces/innerkits/ethernetclient/include/proxy/）
```

## 2. 节点职责表

| # | 节点 | 文件 | 要点 |
|---|---|---|---|
| 1 | HDI 代理 | `libethernet_proxy_1.0`（依赖仓） | 硬件事件入口；接口变更走 HDI 版本评审 |
| 2 | SA 壳 | `src/ethernet_service.cpp` | `OnAddSystemAbility` 依赖就绪门槛：**Conn 1151 与公共事件服务未就绪时不得初始化**（:86/:102） |
| 3 | 管理器 | `src/ethernet_management.cpp`（1131 行） | 口配置/供电/接口表；历史 ABBA 死锁区（30cd0bb，见 [../expert/historical-failures.md](../expert/historical-failures.md)） |
| 4 | 接口状态 | `src/dev_interface_state.cpp` | 供应商注册 :161 / linkInfo 上报 :188；`regState_`/`linkInfo_` 判空后才注册（:188 起日志可见 LINK_UNAVAILABLE 分支） |
| 5 | DHCP | `dhcp_c_api.h`（依赖仓 dhcp） | 结果回调进入状态机更新 IP |
| 6 | 对 base 的出口 | `NetManagerCenter` / `NetConnClient` | **进程内唯一对外连接面**：supplier/linkInfo 经它到 1151 |

## 3. 关键状态与判定

- 接口可用性：`DevInterfaceState` 持 `netSupplier_`/`regState_`/`linkInfo_`；未注册成功前仅记录不上报（`RemoteUpdateNetLinkInfo` 内判空先行，:188 起）。
- 使能/去使能形态面：`netmanager_ext_ethernet_enable_disable`（默认 false）开启后暴露启停命令；条件编译两态可用（[../expert/cross-component-boundaries.md](../expert/cross-component-boundaries.md)）。
- 配置下发（`SetInterfaceConfig` 系）：经 IDL stub 进入 service（[idl-generated-code-map.md](idl-generated-code-map.md) §ethernet），管理器落 DHCP/静态配置。

## 4. 红区与教训

| 文件 | 风险 | 教训 |
|---|---|---|
| `ethernet_management.cpp`（1131 行） | 多接口状态并发更新 × 配置下发 | 30cd0bb：**ABBA 歁锁**——管理锁与 DHCP 回调锁交叉；修复=锁粒度拆分。改此文件必须绑并发回归（[../expert/state-machine-concurrency.md](../expert/state-machine-concurrency.md)） |
| `configuration_parcel_ipc.cpp`（`frameworks/native/ethernetclient/src/`） | Parcel 对称/判空 | 23c57cd：异常分支内存泄漏；IPC 契约见 base 仓 ipc 文档同源规则 |
| `dev_interface_state.cpp` | supplier 生命周期 | 注册与 Dhcp 回调竞态；回调入口判空 |

## 5. 测试绑定

- 单测：`ethernet_manager_test`（`test/ethernetmanager/unittest/`）。
- Fuzzer：`EthernetClientFuzzTest`（`test/fuzztest/`）。
- 连带：改供应商注册语义时按 base 侧连接链评估（base 仓 `net-conn-pipeline-map.md`）。

## 关联知识

- [netmanager-ext-code-map.md](netmanager-ext-code-map.md)：§4 SA/开关基准
- [idl-generated-code-map.md](idl-generated-code-map.md)：`IEthernetService.idl` 与回调生成物
- [../expert/state-machine-concurrency.md](../expert/state-machine-concurrency.md)：锁粒度与回调并发模式
- [../expert/cross-component-boundaries.md](../expert/cross-component-boundaries.md)：对 HDI/DHCP/base 的边界
- [../verify/verification-loop.md](../verify/verification-loop.md)：Done 定义

## 适用场景

- 以太网口配置/插拔事件/DHCP 行为修改
- 供应商注册或 linkInfo 上报口径变更
- 排查"以太网接口注册不到 base/DefaultNet 不生效"类问题
