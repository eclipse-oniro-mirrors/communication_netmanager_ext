# netmanager_ext 总体代码地图

> 本文是 netmanager_ext 仓的全局导航图：给出分层、目录职责、7 SA 总览、特性开关、错误码与权限归属、以及与 netmanager_base/netstack 的边界。任何任务动手前先在此定位"代码在哪一层、归哪个 SA、先读哪篇专题文档"。

## 1. 组件定位与形态

- 组件 `@ohos.netmanager_ext`，SysCap：Ethernet / NetSharing / MDns / Vpn / NetFirewall / NetworkSlice / WearableDistributedNet / EAP（又见 `@ohos.net.eap.d.ts`）。
- 职责：系统网络扩展能力——以太网管理（HDI/DHCP/供电管理）、网络共享（tethering：Wi-Fi 热点共享/USB/BT PAN）、mDNS（协议栈 + 独立进程服务）、VPN（传统 VPN 与分层 VPN、SysVPN）、防火墙与流量过滤、网络切片（URSP）、穿戴分布式网络。
- 形态：**多 SA 系统服务仓**——能力全部以 SA 形态运行在 `netmanager` 或专用进程；无独立引擎层，内核落地经 netmanager_base 的 `NetsysController` 唯一通道。
- 与 netmanager_base 的关系：**SA profile 分工**——1154/1155/1157 的 JSON profile 在 base 仓 `sa_profile/`（本仓另有 1154/1155 的扩展 XML profile），1161/8300/8301/8400 的 profile 在本仓 `sa_profile/`；实现全部在本仓。220 系列错误码与权限常量定义在 base 仓（`interfaces/innerkits/include/net_manager_constants.h`、`utils/common_utils/include/netmanager_base_permission.h`）。
- 观测：事件沿用 domain `NETMANAGER`，`NET_SHARING_*`/`NET_VPN_*`/`NET_MDNS_*`/`NET_FIREWALL_*` 等事件名声明在 **base 仓 `hisysevent.yaml`**（本仓无独立声明文件）。

## 2. 分层架构（自上而下，依赖只能向下）

```
JS API 声明        interfaces/kits/js/@ohos.net.{ethernet,sharing,mdns,eap}.d.ts + basic.d.ts（系统 API 另见 extension series）
   ↓
NAPI 层            frameworks/js/napi/{ethernet,sharing,mdns,vpn,vpnext,netfirewall,extensionability,extensioncontext}
   ↓
客户端（inner API） frameworks/native/{ethernetclient,netshareclient,netvpnclient,mdnsclient,netfirewallclient,
                   networksliceclient,vpnextension,wearabledistributednetclient}
                   + interfaces/innerkits/{...,include}（部分经 IDL 生成：*.idl → *_interface_stub）
   ↓
服务端 SA          services/{ethernetmanager,networksharemanager,vpnmanager,mdnsmanager,
                   netfirewallmanager,networkslicemanager,wearabledistributednetmanager}
   ↓
内核通道（唯一）    services/* → NetsysController（base 仓）→ SA 1158（netsysnative 进程）→ netlink/iptables/BPF
```

- 多语言绑定：`frameworks/ets/ani`（ArkTS Rust ANI，如 sharing 模块 `src/bridge.rs`）、`frameworks/c/net_trafficfilter`（NDK）。同一能力变更需同步绑定（ANI 以 `Cargo.toml` members 组织）。
- 例外形态：`frameworks/vpn_dialog`——VPN 授权弹窗（UI 侧组件，不属分层主链）。

## 3. 二级目录职责表

| 路径 | 职责 | 关键文件 |
|---|---|---|
| `netmanager_ext_config.gni` | 全部特性开关（`declare_args`，见 §5） | — |
| `sa_profile/` | SA 描述：`1154.xml`、`1155.xml`、`1161.json`、`8300.json`、`8301.json`、`8400.json` | — |
| `interfaces/kits/js` | 公共 JS API 声明 | `@ohos.net.ethernet.d.ts`、`@ohos.net.sharing.d.ts`、`@ohos.net.mdns.d.ts`、`@ohos.net.eap.d.ts`、`basic.d.ts` |
| `interfaces/innerkits/ethernetclient` | 以太网 inner API + IDL | `IEthernetService.idl`、`proxy/interface_state_callback_{stub,proxy}.h` |
| `interfaces/innerkits/netshareclient` | 共享 inner API + IDL | `INetworkShareService.idl`、`proxy/ipccallback/sharing_event_callback_{stub,proxy}.h` |
| `interfaces/innerkits/netvpnclient` | VPN inner API + IDL | `INetworkVpnService.idl`、`IVpnEventCallback.idl` |
| `interfaces/innerkits/mdnsclient` | mDNS inner API + IDL | `IMdnsService.idl`、`IDiscoveryCallback.idl`、`IRegistrationCallback.idl`、`IResolveCallback.idl` |
| `interfaces/innerkits/netfirewallclient` | 防火墙 inner API（手写 proxy/stub，无 IDL） | `netfirewall_intercept_record_callback_stub.h`、`i_net_intercept_record_callback.h` |
| `interfaces/innerkits/{networksliceclient,vpnextension,wearabledistributednetclient,include}` | 切片/VPN 扩展/穿戴分布式/公共 | `IWearableDistributedNet.idl` 等 |
| `frameworks/js/napi/*` | 各模块 NAPI exec | `ethernet`、`sharing`、`mdns`、`vpn`、`vpnext`、`netfirewall`、`extensionability`、`extensioncontext` |
| `frameworks/native/*` | 各模块客户端库（proxy/IDL proxy） | 对应 `*client` 目录 |
| `frameworks/ets/ani` | ArkTS↔Rust ANI 绑定 | 各模块 `src/bridge.rs` |
| `frameworks/c/net_trafficfilter` | NDK 头文件 | — |
| `frameworks/vpn_dialog` | VPN 授权弹窗组件 | — |
| `services/ethernetmanager` | SA 1157 服务端 | `src/ethernet_service.cpp`（类声明 `include/ethernet_service.h:39`）、`src/ethernet_management.cpp`（1131 行）、`src/dev_interface_state.cpp` |
| `services/networksharemanager` | SA 1154 服务端 + 状态机 | `src/networkshare_service.cpp`（630 行，类声明 `include/networkshare_service.h:33`）、`src/networkshare_tracker.cpp`（1755 行）、`src/networkshare_main_statemachine.cpp`、`src/networkshare_sub_statemachine.cpp`、`src/router_advertisement_daemon.cpp` |
| `services/vpnmanager` | SA 1155 服务端（传统 + 分层 VPN + SysVPN） | `src/networkvpn_service.cpp`（2960 行，类声明 `include/networkvpn_service.h:84`）、`src/net_vpn_impl.cpp` |
| `services/mdnsmanager` | SA 1161 服务端（独立 mdnsmanager 进程） | `include/mdns_service.h:37`、`src/mdns_protocol_impl.cpp`（1049 行）、`src/protocol/mdns_packet_parser.cpp` |
| `services/netfirewallmanager` | SA 8300 服务端 | `src/netfirewall_service.cpp`（762 行）、`src/nettrafficfilter_redirect_manager.cpp`（1413 行）、`src/netfirewall_rule_manager.cpp`、`src/netfirewall_stub.cpp` |
| `services/networkslicemanager` | SA 8301 服务端（URSP） | `src/hwnetworkslicemanager.cpp`（1361 行）、`src/networkslicemanager.cpp`（1185 行）、`src/urspconfig.cpp`（2148 行） |
| `services/wearabledistributednetmanager` | SA 8400 服务端 | 对应 src |
| `tools/ohos-networkShare` | 网络共享 CLI（需系统调用方） | — |
| `utils/netmanager_ext_common` | 本仓公共工具/权限扩展 | — |
| `services/etc` + 根 `resource/` | init cfg、trust.json、UrspConfig XML 等配置 | `UrspConfig{1,2}.xml` |
| `test` | 单测与 fuzzer（见 §7） | — |

> 行号写作时点已核对；引用前若怀疑漂移，以 `grep -n` 现场核对为准。

## 4. 7 SA 总览

| SA ID | 服务类（声明位置） | 进程 | 特性开关 |
|---|---|---|---|
| 1154 | `NetworkShareService`（`services/networksharemanager/include/networkshare_service.h:33`） | netmanager | `netmanager_ext_feature_share`（默认 true） |
| 1155 | `NetworkVpnService`（`services/vpnmanager/include/networkvpn_service.h:84`） | netmanager | `netmanager_ext_feature_vpn`/`_vpnext`（默认 true）；SysVPN 面 `_sysvpn`、`_vpn_for_user0`（默认 false） |
| 1157 | `EthernetService`（`services/ethernetmanager/include/ethernet_service.h:39`） | netmanager | `netmanager_ext_feature_ethernet`（默认 true）；启停面 `_ethernet_enable_disable`（默认 false） |
| 1161 | `MDnsService`（`services/mdnsmanager/include/mdns_service.h:37`） | **mdnsmanager（独立进程）** | `netmanager_ext_feature_mdns`（默认 true） |
| 8300 | `NetFirewallService`（`services/netfirewallmanager`） | netmanager | `netmanager_ext_feature_net_firewall`（默认 false） |
| 8301 | NetworkSliceService（`services/networkslicemanager`） | netmanager | `netmanager_ext_feature_networkslice`（默认 false） |
| 8400 | WearableDistributedNetService（`services/wearabledistributednetmanager`） | netmanager | `netmanager_ext_feature_wearable_distributed_net`（默认 false） |

- profile 位置：1154/1155 双形态（base JSON + 本仓 `1154.xml`/`1155.xml`）；1157 仅 base JSON；1161/8300/8301/8400 在本仓 JSON。
- IDL 生成边界（生成物不改源）详见 [idl-generated-code-map.md](idl-generated-code-map.md)。

## 5. 特性开关（`netmanager_ext_config.gni` `declare_args`，已核对实名）

| 开关 | 默认 | 说明 |
|---|---|---|
| `netmanager_ext_feature_ethernet` / `_share` / `_mdns` / `_vpn` / `_vpnext` | **true** | 五个主能力默认开启 |
| `enable_netmgr_ext_debug` | true | 调试面 |
| `netmanager_ext_feature_net_firewall` / `_sysvpn` / `_vpn_for_user0` / `_wearable_distributed_net` / `_networkslice` / `_iface_supplier_id` / `_ethernet_enable_disable` | false | 分步骤/形态能力 |
| `netmanager_ext_extensible_authentication`（EAP 扩展认证） | false | — |
| `netmanager_ext_share_notification_enable` / `netmanager_ext_share_traffic_limit_enable` | false | 共享副能力 |
| `netmanager_ext_feature_coverage` / `netmanager_ext_fpga_mode_enable` / `netmanager_ext_feature_read_ccm_json` | false | 形态化开关 |
| 外部协作开关 | — | `communication_bluetooth_switch_enable`、`communication_wifi_switch_enable`、`usb_manager_enable`、`battery_manager_switch_enable`（多默认 false，影响共享底层可用性） |

**开关相关代码必须在开/关两种配置下均编译、均测试**（见 [../verify/verification-loop.md](../verify/verification-loop.md)）。

## 6. 错误码 / 权限 / 观测归属

| 基线 | 定义位置（**都在 base 仓**） | 本仓用法 |
|---|---|---|
| 错误码 220 段（2200001~2200208、2201001~ 以太网、2202xxx 共享、2203~2206 VPN、2204xxx mDNS） | `netmanager_base/interfaces/innerkits/include/net_manager_constants.h` | 本仓只消费不定义，禁止自造裸数字 |
| 权限常量（`CONNECTIVITY_INTERNAL`/`MANAGE_VPN`/`GET_NETWORK_INFO` 等） | `netmanager_base/utils/common_utils/include/netmanager_base_permission.h` | 服务端统一经 `NetManagerPermission::CheckPermission*` 校验 |
| 维测事件（domain `NETMANAGER`：`NET_SHARING_*`/`NET_VPN_*`/`NET_MDNS_*`/`NET_FIREWALL_*`） | `netmanager_base/hisysevent.yaml` | 新事件需在 base 声明、本仓上报 |

本仓特有安全语义：VPN 的 `vpn_dialog` 授权弹窗、EDM 管控（`PERMISSION_MANAGE_EDM_POLICY` 比对 `hasOpenedVpnUid_`），详见 [../expert/permission-security-constraints.md](../expert/permission-security-constraints.md)。

## 7. 测试组织

| 类型 | 实名（已核对 BUILD.gn） |
|---|---|
| 单元测试（`test/<模块>/unittest/`） | `ethernet_manager_test`、`networkshare_manager_test`、`mdns_manager_test`、`mdns_protocol_test`、`netfirewallmanager_test`、`networkslice_manager_test`、`sys_vpn_manager_test`、`vpn_manager_test`、`vpn_manager_ext_test`、`wearable_distributed_net_manager_test` |
| Fuzzer（`test/fuzztest/`） | `EthernetClientFuzzTest`、`NetShareClientFuzzTest`、`MDnsClientFuzzTest`、`VpnClientFuzzTest`、`VpnClientRequestVpnPermissionFuzzTest`、`NetFirewallClientFuzzTest` |
| 聚合 | `//foundation/communication/netmanager_ext/test:netmanager_ext_test` |

## 关联知识

- [ethernet-pipeline-map.md](ethernet-pipeline-map.md)、[vpn-pipeline-map.md](vpn-pipeline-map.md)、[networkshare-pipeline-map.md](networkshare-pipeline-map.md)、[idl-generated-code-map.md](idl-generated-code-map.md)：四大链路与 IDL 边界
- [../routing/task-routing.md](../routing/task-routing.md)：按场景路由
- [../expert/cross-component-boundaries.md](../expert/cross-component-boundaries.md)：对 base/HDI/DHCP 的跨仓边界
- [../verify/build-and-test.md](../verify/build-and-test.md)：构建与测试命令

## 适用场景

- 第一次进入本仓，建立"7 SA + 状态机/协议栈"全局认知
- 不确定某任务归哪个 SA、哪一层、先读哪篇文档
- 评估改动影响面（跨进程/跨仓/开关形态）
