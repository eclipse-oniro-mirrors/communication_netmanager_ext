# netmanager_ext 术语路由表（vocabulary-routing）

> 组件 `@ohos.netmanager_ext` | subsystem `communication` | 任务/场景路由见 [task-routing.md](task-routing.md)
> 看到术语不知道指什么时查本表：术语 → 一句话含义 → 深入位置。含义不确定时**禁止**靠猜改代码。

## 1. 网络共享（share）域

| 术语 | 含义 | 深入位置 |
|---|---|---|
| tethering | 设备把移动数据/以太网分享给他设备的总称（中文＝网络共享） | [../code_map/networkshare-pipeline-map.md](../code_map/networkshare-pipeline-map.md) |
| upstream / downstream | 共享的上行（联网侧）/下行（被分享侧，如热点客户端）网卡 | `networkshare_tracker.cpp`、`networkshare_upstreammonitor.cpp` |
| `FAKE_DOWNSTREAM_IFACENAME` | 主状态机在无真实下行口时的占位网卡名（恢复逻辑用到；`networkshare_main_statemachine.cpp:31`） | [../expert/state-machine-concurrency.md](../expert/state-machine-concurrency.md) |
| `netshareRequester_` / `clientRequestsBitMask_` | tracker 记录共享请求方的容器；位图改 atomic 后修复 UAF（2dd7ac1） | `networkshare_tracker.cpp`、[../expert/historical-failures.md](../expert/historical-failures.md) |
| MainState / SubState | 主状态机（NAT/转发级）与子状态机（按 WIFI/USB/蓝牙各自驱动）两层 | `networkshare_main_statemachine.cpp`、`networkshare_sub_statemachine.cpp` |
| RNDIS | USB 网络共享的网卡协议形态（子状态机 USB 路径） | `networkshare_sub_statemachine.cpp` |
| RA / Router Advertisement | 子状态机起 hub 时对外发路由通告的守护进程（`router_advertisement_daemon.cpp`，f5cc878 mac 判空） | `services/networksharemanager/src/` |
| `EnableNat` / `IpEnableForwarding` | 主状态机开 NAT 与内核 IP 转发的两步（:280/:290、:363） | `networkshare_main_statemachine.cpp` |
| `SharingIfaceType` / `SharingIfaceState` | 共享网卡类型/状态枚举（API 语言侧：`@ohos.net.sharing.d.ts` :491/:458） | `interfaces/kits/js/` |
| wifi_hotspot 复用 | 子状态机 WIFI 路径复用 WiFi 热点能力（`wifi_hotspot.h:35`、热点模式 `:518`） | `networkshare_sub_statemachine.cpp` |

## 2. VPN 域

| 术语 | 含义 | 深入位置 |
|---|---|---|
| `VpnConnection` | JS 应用持有的 VPN 连接对象（@ohos.net.vpn 暴露） | [../code_map/vpn-pipeline-map.md](../code_map/vpn-pipeline-map.md) |
| `hasOpenedVpnUid_` | service 记录已开 VPN 的调用方 uid，二次请求要比对（`networkvpn_service.cpp:945`） | [../expert/permission-security-constraints.md](../expert/permission-security-constraints.md) |
| `setVpnPidMap_` | VPN 进程映射表并发写竞态修复点（4e24608） | `networkvpn_service.cpp` |
| Protect / fwmark | 保护 VPN 自身流量不进隧道：经 base 下内核对 socket 打标记 | `net_vpn_impl.cpp` |
| uid 段路由（split VPN） | 只让指定 uid 段流量进 VPN（`NetworkAddUids`:174/:252/:292，`UpdateVpnRules:367`） | [../code_map/vpn-pipeline-map.md](../code_map/vpn-pipeline-map.md) |
| tunv4 / multitun | 系统 VPN 建的虚拟网卡设备名前缀 | `services/vpnmanager/` |
| `IVpnEventCallback` | VPN 事件回 JS 回调的 IDL 回调接口 | [../code_map/idl-generated-code-map.md](../code_map/idl-generated-code-map.md) |
| SysVPN | 系统 VPN 能力开关 `netmanager_ext_feature_sysvpn`（默认 false） | `netmanager_ext_config.gni` |
| `vpn_dialog` | 三方 VPN 授权弹窗 UI（`frameworks/vpn_dialog`） | [../expert/permission-security-constraints.md](../expert/permission-security-constraints.md) |
| EDM | 企业设备管理（`networkvpn_service.cpp:1068-1069` 走 EDM 接口校验） | [../expert/permission-security-constraints.md](../expert/permission-security-constraints.md) |
| `Prepare` | 三方 VPN 前置请求，service 侧决定是否允许（`networkvpn_service.cpp:322`） | `networkvpn_service.cpp` |

## 3. 以太网域

| 术语 | 含义 | 深入位置 |
|---|---|---|
| `EthernetService` / `EthernetManagement` | SA 1157 门面（:86 订 SA 1151、:102 起 management）与配置内核 | [../code_map/ethernet-pipeline-map.md](../code_map/ethernet-pipeline-map.md) |
| `DevInterfaceState` | 每网卡设备状态对象，向 base 注册 supplier/链路信息（:161/:188） | `dev_interface_state.cpp` |
| NetSupplier / NetLinkInfo | 设备网络能力向 netmanager_base 的注册载体（远程注册 :161、更新 :188） | `dev_interface_state.cpp`、base 仓 netconn |
| 802.1x / EAP | 有线认证协议能力（`@ohos.net.eap.d.ts`） | `interfaces/kits/js/` |
| 使能/禁用开关 | `netmanager_ext_feature_ethernet_enable_disable`（默认 false，控制部分接口编译） | `netmanager_ext_config.gni` |

## 4. mDNS / 防火墙 / 切片 / 穿戴域

| 术语 | 含义 | 深入位置 |
|---|---|---|
| mDNS（.local） | 组播 DNS 局域网发现协议，SA 1161 独立进程 | [../code_map/netmanager-ext-code-map.md](../code_map/netmanager-ext-code-map.md) §4 |
| `mdns_packet_parser` | 报文解析器，畸形报文先校验后拷贝（51e8b7d） | `services/mdnsmanager/src/protocol/` |
| `NetFirewallService` | SA 8300 防火墙（手写 proxy/stub，开关默认 false） | `services/netfirewallmanager` |
| `networkslice` / URSP | 切片策略：`NetSliceService` SA 8301；URSP 规则配置在 `urspconfig.cpp`（2148 行） | `services/networkslicemanager` |
| 穿戴分布式网络 | `WearableDistributedNetService` SA 8400（开关默认 false） | `services/wearabledistributednetmanager` |

## 5. 跨仓通用

| 术语 | 含义 | 深入位置 |
|---|---|---|
| `NetsysController` | **base 仓**侧内核操作唯一通道；ext 一切内核动作经它 | [../expert/cross-component-boundaries.md](../expert/cross-component-boundaries.md) |
| 错误码 220 段 / 权限常量 / 观测事件 | 全部定义在 **base 仓**（`netmanager_base_error.h`、`netmanager_base_permission.h`、hisysevent domain NETMANAGER） | [../expert/cross-component-boundaries.md](../expert/cross-component-boundaries.md) |
| IDL 生成面 / 手写面 | `.idl` 编译期生成（不落仓）vs netfirewall/networkslice 手写 proxy/stub | [../code_map/idl-generated-code-map.md](../code_map/idl-generated-code-map.md) |
| `NetConnClient` | base 仓网络连接客户端，ext 用来取网络与注册 supplier | base 仓 `frameworks/native/netconnclient` |

## 关联知识

- 场景路由：[task-routing.md](task-routing.md)
- 全仓总图：[../code_map/netmanager-ext-code-map.md](../code_map/netmanager-ext-code-map.md)

## 适用场景

- 评审/读码遇到陌生标识符（如 `FAKE_DOWNSTREAM_IFACENAME`、fwmark）需要快速对含义；
- 写文档/提交信息需要统一术语叫法（tethering vs 网络共享）；
- 判断某术语归属本仓还是 base 仓，避免跨仓改错位置。
