# 网络共享链路图（SA 1154）

> 本文描述 tethering 全链路：CLI/JS → 服务 → tracker → 主/子状态机 → NAT 转发落地，以及 upstream 网络选择与 RA 守护。改 `services/networksharemanager`、`frameworks/native/netshareclient`、`frameworks/js/napi/sharing` 前必读。本模块是 ext 仓**状态机最复杂、历史失效最多**的链路。

## 1. 总链路（下游共享 + 上游选择）

```
入口①：tools/ohos-networkShare CLI（系统调用方）
入口②：JS @ohos.net.sharing（frameworks/js/napi/sharing）
   ↓
NetworkShareClient（frameworks/native/netshareclient）→ IDL proxy（INetworkShareService.idl）
   ↓
NetworkShareService（services/networksharemanager/src/networkshare_service.cpp，630 行）
   ├─ 权限门：CheckPermission(Permission::CONNECTIVITY_INTERNAL)（:175 起 10+ 处）
   ↓
NetworkShareTracker（src/networkshare_tracker.cpp，1755 行红区）
   ├─ 客户端请求位图 clientRequestsBitMask_（atomic；2dd7ac1 由 vector 改造）
   ↓
主状态机 NetworkShareMainStateMachine（src/networkshare_main_statemachine.cpp）
   ├─ IpEnableForwarding(netshareRequester_)(:363) → base NetsysController
   ├─ EnableNat(FAKE_DOWNSTREAM_IFACENAME(:31), upstreamIfaceName_)(:280、:290)
   └─ 关闭：DisableNat / IpDisableForwarding(:399、:401)
   ↓
子状态机 NetworkShareSubStateMachine（src/networkshare_sub_statemachine.cpp）
   ├─ WIFI 热点共享 / USB（RNDIS）/ BT PAN（复用 wifi_hotspot 形态，:518 附近；wifi_hotspot.h :35）
   └— DHCP 服务端、接口启停
   ↓ upstream 选择
UpstreamMonitor（src/networkshare_upstreammonitor.cpp）
   ├— NetConnClient::RegisterNetConnCallback 订阅 base 网络变化（回调判空 ：115/:178 等）
   └— RA 守护 router_advertisement_daemon.cpp（IPv6 RA 下发；f5cc878：RA 源 MAC 判空）
```

## 2. 节点职责表

| # | 节点 | 文件 | 要点 |
|---|---|---|---|
| 1 | 权限门 | `networkshare_service.cpp:175` 起 | 全部管理命令逐条 `CONNECTIVITY_INTERNAL`；CLI 需系统调用方 + EDM 管控语义（[../expert/permission-security-constraints.md](../expert/permission-security-constraints.md)） |
| 2 | 请求位图 | `networkshare_tracker.cpp` | `clientRequestsBitMask_` **atomic 位图**（2dd7ac1 教训：vector 并发 UAF）；修改请求聚合必须保持无锁读 |
| 3 | 主状态机 | `networkshare_main_statemachine.cpp` | `MainState` 三态（INIT/ALIVE/ERROR）；转发/NAT 用假下游名 `FAKE_DOWNSTREAM_IFACENAME`(:31) |
| 4 | 子状态机 | `networkshare_sub_statemachine.cpp` | `SubState`（INIT/SHARED/RESTART/UNAVAILABLE）× 三种下游介质；BT PAN 依赖蓝牙开关 |
| 5 | upstream | `networkshare_upstreammonitor.cpp` | 网络变化→重选上游→重挂 NAT；**回调判空**（ec54b35 教训） |
| 6 | RA 守护 | `router_advertisement_daemon.cpp` | 下发 RA；MAC 为空的异常分支必须防御（f5cc878） |
| 7 | 内核落地 | base `NetsysController` | NAT/转发唯一通道 |

## 3. 状态机与枚举（JS 口径在 `@ohos.net.sharing.d.ts`）

| 枚举 | 位置 | 用途 |
|---|---|---|
| `SharingIfaceType` | `@ohos.net.sharing.d.ts:491` | NONE/WIFI/USB/BLUETOOTH 下游介质 |
| `SharingIfaceState` | `@ohos.net.sharing.d.ts:458` | SERVING/CAN_SERVER/ERROR 共享状态 |
| `MainState`/`SubState` | `networkshare_{main,sub}_statemachine*.h` | 内部双层状态机；事件驱动迁移 |

**事件风暴**：批量开关接口时事件常被打散，务必按幂等设计（重复 `EnableNat`/`IpEnableForwarding` 不产生副作用）。

## 4. 红区与教训（本模块 TOP 失效区）

| hash | 现象→根因→修复 | 验证要点 |
|---|---|---|
| 2dd7ac1 | 多线程请求 tracker 崩溃/UAF → `clientRequestsVector_` 无锁 → 改 `clientRequestsBitMask_` atomic | 并发 500 次请求压测 |
| 5842ca2 | crash 恢复后被关闭的热点重开 → 恢复逻辑误判客户端请求 → 剔除误开分支 | kill -9 后状态恢复检查 |
| ec54b35 | upstream 回调空指针崩溃 → `new NetConnectionCallback` 结果/回调未判空 → 全面判空 | 网络频繁切换压测 |
| f5cc878 | RA 偶现 mac 空指针 → 异常分支未防御 → 判空 | 畸形接口场景 |

台账详见 [../expert/historical-failures.md](../expert/historical-failures.md)；并发模式见 [../expert/state-machine-concurrency.md](../expert/state-machine-concurrency.md)。

## 5. 测试绑定

- 单测：`networkshare_manager_test`（含状态机用例）。
- Fuzzer：`NetShareClientFuzzTest`。
- 观测：`NET_SHARING_SETUP_FAULT`/`NET_SHARING_CANCEL_FAULT`（domain `NETMANAGER`，声明在 base 仓）。

## 关联知识

- [netmanager-ext-code-map.md](netmanager-ext-code-map.md)：§4 SA / §5 开关（含蓝牙/USB 协作开关）
- [idl-generated-code-map.md](idl-generated-code-map.md)：`INetworkShareService.idl` 与 `sharing_event_callback_{stub,proxy}`
- [../expert/state-machine-concurrency.md](../expert/state-machine-concurrency.md)：状态机并发模式一/模式二
- [../verify/verification-loop.md](../verify/verification-loop.md)：Done 定义

## 适用场景

- 共享开关/介质（Wi-Fi/USB/BT）行为修改
- 排查"共享开了没网/恢复后状态错乱/回调崩溃"
- upstream 重选、RA 下发、NAT 规则评审
