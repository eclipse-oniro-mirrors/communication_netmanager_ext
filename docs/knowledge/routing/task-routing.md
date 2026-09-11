# netmanager_ext 任务路由总表（task-routing）

> 组件 `@ohos.netmanager_ext` | subsystem `communication` | 知识体系四层结构与入口见 [AGENTS.md](../../../AGENTS.md)
> 本篇职责单一：接到任务后**第一跳索引**——按任务类型、按现象、按 SA、按调用方四张表定位去向；命中后再读目标文档。

## 1. 按任务类型路由（先看这里）

| 任务类型 | 先看 | 原因 |
|---|---|---|
| 以太网（使能/禁用、IP 模式、802.1x/EAP） | [../code_map/ethernet-pipeline-map.md](../code_map/ethernet-pipeline-map.md) | HDI→`ethernet_service`（SA 1157）→`ethernet_management`（1131 行红区）→`dev_interface_state`→DHCP 全链 |
| 网络共享 tethering（WiFi 热点/USB RNDIS/BT PAN、NAT） | [../code_map/networkshare-pipeline-map.md](../code_map/networkshare-pipeline-map.md) | `networkshare_service`→`networkshare_tracker`（竞态第一高发区）→主/子状态机→upstream |
| VPN（三方/系统 VPN、EDM 管控、uid 段路由） | [../code_map/vpn-pipeline-map.md](../code_map/vpn-pipeline-map.md) | `networkvpn_service.cpp`（2960 行红区）+ `net_vpn_impl.cpp`（uid 段路由/fwmark） |
| mDNS（发现/注册/解析、.local 协议栈） | [../code_map/netmanager-ext-code-map.md](../code_map/netmanager-ext-code-map.md) §4 + `services/mdnsmanager` | SA 1161 独立进程；核心 `mdns_protocol_impl.cpp`（1049 行）与 `mdns_packet_parser.cpp` |
| 防火墙（拦截规则、iptables 联动） | [../code_map/netmanager-ext-code-map.md](../code_map/netmanager-ext-code-map.md) §4 + `services/netfirewallmanager` | 手写 proxy/stub（非 IDL 生成），SA 8300；开关 `netmanager_ext_feature_net_firewall` 默认 false |
| 网络切片/URSP | `services/networkslicemanager` | 手写面，SA 8301；配置核心 `urspconfig.cpp`（2148 行）、`hwnetworkslicemanager.cpp`（1361 行） |
| 穿戴分布式网络 | `services/wearabledistributednetmanager` + `frameworks/native/wearabledistributednetclient` | SA 8400；开关 `netmanager_ext_feature_wearable_distributed_net` 默认 false |
| 新增 IPC 命令/回调；判断某文件是 IDL 生成还是手写 | [../code_map/idl-generated-code-map.md](../code_map/idl-generated-code-map.md) | 9 个 `.idl` 清单 + 30 秒判别法 + 最小动作集 |
| JS/NDK/ArkTS 公共 API 变更 | [../expert/cross-component-boundaries.md](../expert/cross-component-boundaries.md) §5 | `interfaces/kits/js/*.d.ts` 基线与 `frameworks/{js,c,ets,native}` 多套绑定成套改 |
| 崩溃/UAF/竞态/死锁排查 | [../expert/historical-failures.md](../expert/historical-failures.md) | 10 个历史 hash 台账（现象/根因/落点文件/验证要点） |
| 状态机事件回调/并发正确性 | [../expert/state-machine-concurrency.md](../expert/state-machine-concurrency.md) | 主/子状态机事件风暴、锁序与恢复序模式 |
| 权限/调用方身份（uid/pid/EDM）校验 | [../expert/permission-security-constraints.md](../expert/permission-security-constraints.md) | `CONNECTIVITY_INTERNAL`、`MANAGE_VPN`、`hasOpenedVpnUid_` 检查点清单 |
| 改动波及 base/netsys/HDI/蓝牙/WiFi 等外部 | [../expert/cross-component-boundaries.md](../expert/cross-component-boundaries.md) | 内核通道唯一经 base `NetsysController`；错误码/权限常量/观测全在 base 仓 |
| 选编译目标/选测试 | [../verify/build-and-test.md](../verify/build-and-test.md) | 单测 10 + fuzz 6 实名表 + 开关矩阵 + 红区大文件表 |
| 验证顺序/Done 定义/回退方案 | [../verify/verification-loop.md](../verify/verification-loop.md) | 六步验证循环 + 提交前检查清单 |
| 都不像 | 先回 [../code_map/netmanager-ext-code-map.md](../code_map/netmanager-ext-code-map.md) 目录职责表 | 表驱动兜底，禁止盲改 |

## 2. 按现象快查（症状 → 高概率位置 → 历史案例）

> 各案例根因与修复要点详见 [../expert/historical-failures.md](../expert/historical-failures.md)。

| 现象 | 高概率位置 | 历史案例 |
|---|---|---|
| 进程 crash 恢复后，无连接的热点被重新打开 | `services/networksharemanager/src/networkshare_service.cpp` + `networkshare_tracker.cpp` 恢复路径 | 5842ca2 |
| 共享客户端回调集合 UAF/位图竞态 | `networkshare_tracker.cpp`（`netshareRequester_`/`clientRequestsBitMask_` atomic） | 2dd7ac1 |
| 配置线程互锁死锁（ABBA） | `services/ethernetmanager/src/ethernet_management.cpp` | 30cd0bb |
| VPN 崩溃后子进程未清理 | `services/vpnmanager/src/networkvpn_service.cpp` 子进程管理 | 4f6fff0 |
| RA 守护 mac 地址空指针崩溃 | `services/networksharemanager/src/router_advertisement_daemon.cpp` | f5cc878 |
| upstream 回调未判空崩溃 | `networkshare_upstreammonitor.cpp`（:115/:178/:187/:196 共 4 处判空） | ec54b35 |
| 收到畸形 mDNS UDP 报文后内存增长 | `services/mdnsmanager/src/protocol/mdns_packet_parser.cpp` 解析边界（先校验后拷贝） | 51e8b7d |
| IPC 序列化异常分支内存泄漏 | `frameworks/native/ethernetclient/src/{configuration_parcel_ipc,interface_configuration,mac_address_info}.cpp`、`frameworks/native/mdnsclient/src/mdns_service_info.cpp` | 23c57cd |
| `setVpnPidMap_` 并发写竞态 | `networkvpn_service.cpp` | 4e24608 |
| SA 重启后 VPN 客户端收不到事件 | `frameworks/native/netvpnclient/src/networkvpn_client.cpp`（`OnAddSystemAbility:167`） | 124a016 |

## 3. 按 SA 路由（服务类与 profile 实位）

| SA ID | 服务类（声明处） | profile（进程归属以 profile 为准） |
|---|---|---|
| 1154 | `NetworkShareService`（services/networksharemanager/include/networkshare_service.h:33） | 本仓 `sa_profile/1154.xml`（base 仓另有同名 `.json`） |
| 1155 | `NetworkVpnService`（services/vpnmanager/include/networkvpn_service.h:84） | 本仓 `sa_profile/1155.xml`（base 仓另有同名 `.json`） |
| 1157 | `EthernetService`（services/ethernetmanager/include/ethernet_service.h:39） | **仅 base 仓** `sa_profile/1157.json`，本仓无 |
| 1161 | `MDnsService`（services/mdnsmanager/include/mdns_service.h:37） | 本仓 `sa_profile/1161.json`（mdnsmanager 独立进程） |
| 8300 | `NetFirewallService` | 本仓 `sa_profile/8300.json` |
| 8301 | `NetworkSliceService` | 本仓 `sa_profile/8301.json` |
| 8400 | `WearableDistributedNetService` | 本仓 `sa_profile/8400.json` |

⚠️ 1154/1155 的 profile **跨仓双份**（base `.json` + ext `.xml`）：改 SA 注册行为（进程名/onDemand）必须两仓同步核对，只改一边会出现注册不一致。

## 4. 按调用方路由（谁在用 ext）

| 调用方 | 入口位置 |
|---|---|
| JS 应用（@ohos.net.ethernet/sharing/vpn/mdns/eap…） | `frameworks/js/napi/{ethernet,sharing,vpn,vpnext,mdns,netfirewall,extensionability,extensioncontext}`；API 基线在 `interfaces/kits/js/*.d.ts` |
| native 系统调用方 | `frameworks/native/{ethernetclient,netshareclient,netvpnclient,mdnsclient,netfirewallclient,networksliceclient,vpnextension,wearabledistributednetclient}` |
| C/NDK 绑定 | `frameworks/c`（NETSYS 相关 inner API 由 base 仓提供） |
| CLI 排查 | `tools/ohos-networkShare`（系统调用方限权见 [../expert/permission-security-constraints.md](../expert/permission-security-constraints.md)） |
| VPN 授权弹窗 | `frameworks/vpn_dialog`（三方 VPN 授权 UI） |

## 5. 编辑前置（不可跳过）

动手改任何文件前，先按 [AGENTS.md](../../../AGENTS.md) §5 完成三项声明（任务类别 / 已读文档 / 适用约束），表中「先看」文档即对应任务的必读项。不确定归类的任务按 §1 兜底行处理，禁止跳读直接改代码。

## 关联知识

- 兄弟路由：[vocabulary-routing.md](vocabulary-routing.md)（术语反查）
- 总图：[../code_map/netmanager-ext-code-map.md](../code_map/netmanager-ext-code-map.md)
- 台账：[../expert/historical-failures.md](../expert/historical-failures.md)

## 适用场景

- 新接需求/缺陷单，需要确定改动落点与先读文档时；
- 复现现象已有，需要按症状直跳历史案例定位热点文件时；
- 查询某 SA 的服务类、profile 实位与跨仓注意点时。
