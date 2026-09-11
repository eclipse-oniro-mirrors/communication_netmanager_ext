# VPN 管理链路图（SA 1155）

> 本文描述传统 VPN 与分层（split/uid 段路由）VPN 全链路：JS → 客户端 → IDL → `NetworkVpnService` → `NetVpnImpl` → 内核规则。改 `services/vpnmanager`、`frameworks/native/netvpnclient`、`frameworks/js/napi/vpn` 前必读。

## 1. 总链路

```
JS @ohos.net.vpn（frameworks/js/napi/vpn）
   ↓
VpnConnection（frameworks/…/vpn_connection.cpp，应用侧句柄）
   ↓
NetworkVpnClient（frameworks/native/netvpnclient/src/networkvpn_client.cpp）
   ├─ SA 1155 状态订阅：SubscribeSystemAbility(:153)/UnSubscribe(:162)/OnAddSystemAbility(:167)
   │   （124a016：配置保存在 client 侧，service 崩溃重启后重新注册恢复）
   ↓ 经 IDL proxy（INetworkVpnService.idl → networkvpn_interface_stub）
NetworkVpnService（services/vpnmanager/src/networkvpn_service.cpp，2960 行红区）
   ├─ Prepare(:322)：授权体验（是否有已存在 VPN/授权包名）
   ├─ 权限与互斥门：CheckPermission（MANAGE_VPN :689 等）/ EDM 管控（:1068）
   ├─ SetUpVpn：序列化转发(:576) → 实现(:880)；记录 callingUid :933、hasOpenedVpnUid_ = GetCallingUid(:945)
   ↓
NetVpnImpl（services/vpnmanager/src/net_vpn_impl.cpp）
   ├─ NetworkAddUids(:174)（单 uid）/ uid 段路由(:252、:292)：加入 VpnNetwork
   ├─ UpdateVpnRules(:367)：规则增量（isUp 上下线）
   └─ Protect：VPN socket 经 fwmark 保护不经隧道
   ↓
NetsysController（base 仓，唯一内核通道）→ SA 1158（iptables/路由落地）
```

## 2. 节点职责表

| # | 节点 | 文件 | 要点 |
|---|---|---|---|
| 1 | NAPI/应用侧 | `frameworks/js/napi/vpn`、`vpn_connection.cpp` | 生命周期与 JS 回调切线程（同 netstack 模式） |
| 2 | 客户端 | `networkvpn_client.cpp` | **SA 死亡恢复**：订阅 :153-189；配置存 client 侧（124a016） |
| 3 | IDL proxy/stub | `INetworkVpnService.idl` + `IVpnEventCallback.idl`（生成物） | 契约变更先改 .idl（[idl-generated-code-map.md](idl-generated-code-map.md)） |
| 4 | 服务端门禁 | `networkvpn_service.cpp` Prepare:322 | 授权弹窗 `frameworks/vpn_dialog`；权限/系统调用/EDM 三道门（[../expert/permission-security-constraints.md](../expert/permission-security-constraints.md)） |
| 5 | 单 VPN 互斥 | hasOpenedVpnUid_（:945 写、:1069 比对 EDM 门） | 同一时间仅一个应用持有 VPN；非持有者操作被拒 |
| 6 | 分层 VPN | `net_vpn_impl.cpp` | uid/uid 段加入指定 Network 和 priority（:174/:252/:292），规则更新 :367 |
| 7 | 内核落地 | base `NetsysController` | **禁止绕过**（架构不变量，见 [../expert/cross-component-boundaries.md](../expert/cross-component-boundaries.md)） |

## 3. 多 VPN / uid 隔离语义

- 传统 VPN：全设备流量进隧道；网内接口 `tunv4` 形态。
- 分层 VPN：仅指定 uid（或 uid 段）流量进对应 VpnNetwork；多 VPN 共存时要按 `netId_` 添加规则，冲突以 priorityId 决定。
- **VPN 子进程清理**：主进程死亡必须清理已拉起的子进程（4f6fff0，`networkvpn_service.cpp`）；评审任何"新增子进程/守护"改动时核对这条。

## 4. 红区与教训

| 文件 | 风险 | 教训 |
|---|---|---|
| `networkvpn_service.cpp`（2960 行） | uid/互斥状态并发 | 4e24608：`setVpnPidMap_` 多线程冲突；4f6fff0：崩溃后子进程残留。改此文件绑并发回归 |
| `networkvpn_client.cpp` | SA 重启状态丢失 | 124a016：client 侧持久化 + 重订阅；勿把恢复逻辑只放 service 侧 |
| `net_vpn_impl.cpp` | uid 段计算整型溢出/边界 | 段合并与空段判定要绑定单测 |
| EDM/系统应用语义 | 越权 | :1068-1069 EDM 管控与 `hasOpenedVpnUid_` 比对顺序敏感，勿调换 |

## 5. SysVPN 与扩展面

- `netmanager_ext_feature_sysvpn` / `_vpn_for_user0`（默认 false）控制 SysVPN 面；`frameworks/native/vpnextension` + `frameworks/js/napi/vpnext` 是扩展 API 面，CLI 系统调用方语义见 [../expert/permission-security-constraints.md](../expert/permission-security-constraints.md)。
- 开关门控的 IPC 分支两态都要编译测试（[../verify/verification-loop.md](../verify/verification-loop.md)）。

## 6. 测试绑定

- 单测：`vpn_manager_test`、`vpn_manager_ext_test`、`sys_vpn_manager_test`。
- Fuzzer：`VpnClientFuzzTest`、`VpnClientRequestVpnPermissionFuzzTest`。
- 排障观测：`NET_VPN_CONNECT_FAULT`、`VPN_OPERATE_FAULT`（domain `NETMANAGER`，声明在 base 仓）。

## 关联知识

- [netmanager-ext-code-map.md](netmanager-ext-code-map.md)：§4 SA / §5 开关
- [idl-generated-code-map.md](idl-generated-code-map.md)：VPN IDL 与事件回调
- [../expert/permission-security-constraints.md](../expert/permission-security-constraints.md)：授权/EDM/系统调用门
- [../expert/state-machine-concurrency.md](../expert/state-machine-concurrency.md)：互斥状态并发模式
- [../expert/historical-failures.md](../expert/historical-failures.md)：4e24608/4f6fff0/124a016 台账

## 适用场景

- 新增/修改 VPN 配置、连接、分层路由能力
- 排查"连不上/互斥失效/崩溃后残留子进程"
- 授权弹窗、EDM 管控、SysVPN 开关形态的评审
