# IDL 生成代码边界图

> 本仓接口面分两类：**IDL 生成契约**（改 `.idl` 源，生成物编译期产出、不落仓）与**手写 proxy/stub**（直接改 cpp/h）。混着改=改错文件。本文给出两类清单与判别方法。

## 1. 生成契约面（`.idl` 源 → 编译期生成 stub/proxy 代码）

仓库内只有 `.idl` 源与 BUILD 声明；生成物（`*_interface_stub` 系）在 **out 目录编译期产出**，仓库搜不到其源码是正常现象（已核实仓内无 `*interface_stub*` 文件、无 AUTO-GENERATED 标记文件）。

| 模块 | .idl 源（interfaces/innerkits/…） | BUILD 声明形态 |
|---|---|---|
| ethernet | `ethernetclient/IEthernetService.idl` | `sources = [ "IEthernetService.idl" ]` |
| share | `netshareclient/INetworkShareService.idl` | 同上形态 |
| vpn | `netvpnclient/INetworkVpnService.idl` + `netvpnclient/IVpnEventCallback.idl`（`sources_callback` 单列） | `sources`/`sources_callback` |
| mdns | `mdnsclient/IMdnsService.idl`、`IDiscoveryCallback.idl`、`IRegistrationCallback.idl`、`IResolveCallback.idl`（服务 + 三个回调契约） | 同上形态 |
| wearable | `wearabledistributednetclient/IWearableDistributedNet.idl` | 同上形态 |

**规则 IDL-R1**：契约变更（加命令/改参数/加回调）**只改 `.idl` 文件**，生成物禁止手工编辑；编译后生成 API 变化需同步 proxy 侧调用与服务端 OnRemoteRequest 覆盖。
**规则 IDL-R2**：`.idl` 是对外基线的一部分——接口增删转 [../expert/cross-component-boundaries.md](../expert/cross-component-boundaries.md) 的基线核对流程（JS/NDK/ANI 多语言面）。

## 2. 手写 IPC 面（直接改源码）

| 面域 | 手写文件（已核实） | 说明 |
|---|---|---|
| netfirewall | `interfaces/innerkits/netfirewallclient/include/netfirewall_proxy.h`、`services/netfirewallmanager/src/netfirewall_stub.cpp`（622 行）、`include/netfirewall_intercept_record_callback_stub.h`、`include/i_net_intercept_record_callback.h` | 无 IDL，契约靠手写 Parcel 对称（applicable base 仓 ipc 红线：写读逐字段对称+判空） |
| networkslice | `interfaces/innerkits/networksliceclient/include/networkslice_proxy.h` + services 侧 stub | 同上 |
| ethernet 回调 | `ethernetclient/include/proxy/interface_state_callback_{stub,proxy}.h` | 接口状态事件成对 |
| share 回调 | `netshareclient/include/proxy/ipccallback/sharing_event_callback_{stub,proxy}.h`、`isharing_event_callback.h` | 共享事件成对 |
| mdns 回调 | `mdnsclient` 下 IDiscoveryCallback/IRegistrationCallback/IResolveCallback（生成侧） | 与 §1 区分：mDNS 回调走 IDL |

## 3. 判别方法（30 秒定位"该改哪")

```
grep -n "sources.*\.idl" interfaces/innerkits/<module>/BUILD.gn
├─ 命中 → 本模块契约面是 IDL：改 .idl，不动手写文件；生成 stub/proxy 类名在编译产物里
└─ 未命中 → 手写面：直接改 *_proxy.h / *_stub.cpp，逐字段 Parcel 对称照 base 仓 IPC 契约
```

另有两处易混点：

- **回调两态**：同一模块可能"主契约 IDL + 事件回调手写"并存（如 ethernet：`IEthernetService.idl` 生成主面，`interface_state_callback_*` 手写）——按文件名判断，不按模块一刀切。
- **`frameworks/native/<module>` 客户端库**：调用生成 proxy 或持有手写 proxy 的胶水层，两边都可能改（组包细节在自己代码里时）。

## 4. 新增一条命令的最小动作集

1. IDL 面：改 `X.idl`（加方法/入参出参）→ 编译生成新 stub 虚方法 → 服务端 `*Service` 类实现新方法 → 客户端调新生成 proxy 方法。
2. 手写面：接口头加虚方法 + proxy 组包 + stub 分支 + 错误码（220 段，取自 base `net_manager_constants.h`）+ fuzzer case（[../verify/verification-loop.md](../verify/verification-loop.md) §2）。
3. 任一面新增后：JS/NDK/ANI 三面口径核对（[../expert/cross-component-boundaries.md](../expert/cross-component-boundaries.md)）。

## 5. 测试绑定

- IDL 面各模块 client fuzzer：`EthernetClientFuzzTest`/`NetShareClientFuzzTest`/`MDnsClientFuzzTest`/`VpnClientFuzzTest`；手写防火墙面 `NetFirewallClientFuzzTest`。
- 契约变更后至少跑所属模块单测（[netmanager-ext-code-map.md](netmanager-ext-code-map.md) §7 实名表）。

## 关联知识

- [netmanager-ext-code-map.md](netmanager-ext-code-map.md)：目录与模块总图
- [../expert/permission-security-constraints.md](../expert/permission-security-constraints.md)：stub 层权限门
- [../expert/cross-component-boundaries.md](../expert/cross-component-boundaries.md)：多语言基线核对
- base 仓 `docs/knowledge/routing/ipc-extension-routing.md`：手写 proxy/stub 的 8 步 checklist（同源规则）

## 适用场景

- 给任一 ext SA 加接口/回调前的"改哪类文件"判定
- 评审发现有人直接改生成物（或把 .idl 当注释文件）时的纠偏依据
- 排查"加了 .IDL 方法但服务端没生效"（多半是漏实现生成 stub 虚方法）
