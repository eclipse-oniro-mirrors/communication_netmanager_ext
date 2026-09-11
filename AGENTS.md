# AGENTS.md — netmanager_ext 领域知识入口（AI Agent 必读）

> 组件 `@ohos.netmanager_ext` | subsystem `communication` | SysCap 以 `interfaces/kits/js/*.d.ts`（`@ohos.net.ethernet`/`@ohos.net.sharing`/`@ohos.net.vpn`/`@ohos.net.mdns`/`@ohos.net.eap`…）声明为准 | hisysevent 事件归属 base 仓 domain `NETMANAGER`
> 本文件是本仓 AI Agent 工作的第一入口。领域知识库位于 `docs/knowledge/`（13 篇，见 §3）。**编辑任何代码前必须完成 §5 编辑前声明。**

## 1. 项目定位

- 职责一句话：为系统与应用提供**以太网管理、网络共享（tethering）、VPN、mDNS、防火墙、网络切片（URSP）、穿戴分布式网络**七大扩展网络能力。
- 定位：**多 SA 服务扩展仓**——共 7 个 SystemAbility：`NetworkShareService`(1154，`networkshare_service.h:33`)、`NetworkVpnService`(1155，`networkvpn_service.h:84`)、`EthernetService`(1157，`ethernet_service.h:39`)、`MDnsService`(1161，`mdns_service.h:37`，独立 mdnsmanager 进程)、`NetFirewallService`(8300)、`NetworkSliceService`(8301)、`WearableDistributedNetService`(8400)。profile 在本仓 `sa_profile/`（1154.xml/1155.xml/1161.json/8300.json/8301.json/8400.json；**1157 仅在 base 仓，1154/1155 两仓双份**）。
- 与 netmanager_base 的关系：**内核动作唯一经 base `NetsysController`**；错误码 220 段、权限常量（`netmanager_base_permission.h`）、hisysevent 事件全部定义在 base 仓；以太网口经 netconn supplier 体系注册网络。
- 特性开关集中在 `netmanager_ext_config.gni`：默认开 `ethernet`/`share`/`mdns`/`vpn`/`vpnext`；默认关 `net_firewall`/`sysvpn`/`vpn_for_user0`/`wearable_distributed_net`/`networkslice`/`iface_supplier_id`/`ethernet_enable_disable` 等；另有外部硬件开关（wifi/bluetooth/usb/battery）。全表见 [docs/knowledge/verify/build-and-test.md](docs/knowledge/verify/build-and-test.md) §4。
- 分层（自上而下，禁止跨层耦合）：
  1. API 基线：`interfaces/kits/js/*.d.ts`；
  2. 绑定层：`frameworks/js/napi/{ethernet,sharing,vpn,vpnext,mdns,netfirewall,extensionability,extensioncontext}`、`frameworks/c`、`frameworks/ets/ani`（ani_package）、`frameworks/vpn_dialog`（授权弹窗）；
  3. native 客户端（innerkits 交付 SO）：`interfaces/innerkits/{ethernetclient,mdnsclient,netfirewallclient,netshareclient,netvpnclient,networksliceclient,vpnextension,wearabledistributednetclient}`；
  4. 服务层：`services/{ethernetmanager,networksharemanager,vpnmanager,mdnsmanager,netfirewallmanager,networkslicemanager,wearabledistributednetmanager}`；
  5. IPC：9 个 `.idl` 编译期生成 stub/proxy（不落仓）+ netfirewall/networkslice 手写面；
  6. 底座：HDI（`libethernet_proxy_1.0`）、DHCP 仓、base 仓内核通道。
- 工具与配置：CLI `tools/ohos-networkShare`；init 配置与 URSP XML 在 `services/etc/init/`。

## 2. 任务到路径映射

| 任务类型 | 先看这里 | 原因 |
|---|---|---|
| 以太网（使能/IP 模式/802.1x） | [docs/knowledge/code_map/ethernet-pipeline-map.md](docs/knowledge/code_map/ethernet-pipeline-map.md) | HDI→SA 1157→`ethernet_management` 全链 |
| 网络共享/NAT/状态机 | [docs/knowledge/code_map/networkshare-pipeline-map.md](docs/knowledge/code_map/networkshare-pipeline-map.md) | 竞态第一高发区 tracker+主/子状态机 |
| VPN/授权/EDM | [docs/knowledge/code_map/vpn-pipeline-map.md](docs/knowledge/code_map/vpn-pipeline-map.md) | `networkvpn_service.cpp`（2960 行）红区 |
| mDNS/防火墙/切片/穿戴 | [docs/knowledge/code_map/netmanager-ext-code-map.md](docs/knowledge/code_map/netmanager-ext-code-map.md) §4 | SA 1161/8300/8301/8400 各域入口 |
| 新增 IPC 命令/回调 | [docs/knowledge/code_map/idl-generated-code-map.md](docs/knowledge/code_map/idl-generated-code-map.md) | IDL vs 手写判定 + 最小动作集 |
| 不确定任务归类 | [docs/knowledge/routing/task-routing.md](docs/knowledge/routing/task-routing.md) | 场景路由总表（按任务/现象/SA/调用方四表） |
| 看到术语不知道含义 | [docs/knowledge/routing/vocabulary-routing.md](docs/knowledge/routing/vocabulary-routing.md) | 术语反查表 |
| 崩溃/UAF/竞态/死锁 | [docs/knowledge/expert/historical-failures.md](docs/knowledge/expert/historical-failures.md) | 10 个历史 hash 台账 |
| 状态机/并发/SA 恢复 | [docs/knowledge/expert/state-machine-concurrency.md](docs/knowledge/expert/state-machine-concurrency.md) | 并发域图 + 对称/幂等/恢复序规则 |
| 权限/uid/EDM 校验 | [docs/knowledge/expert/permission-security-constraints.md](docs/knowledge/expert/permission-security-constraints.md) | 检查点实名清单 |
| 跨仓/base/HDI/绑定同步 | [docs/knowledge/expert/cross-component-boundaries.md](docs/knowledge/expert/cross-component-boundaries.md) | 内核唯一通道等边界铁律 |
| 编译目标/测试选择 | [docs/knowledge/verify/build-and-test.md](docs/knowledge/verify/build-and-test.md) | 域组+单测 10+fuzz 6 实名 |
| 验证顺序/Done/回退 | [docs/knowledge/verify/verification-loop.md](docs/knowledge/verify/verification-loop.md) | 六步循环检查单 |

## 3. 知识索引（场景路由）

| 场景 | 先读 |
|---|---|
| 了解全仓分层/目录/SA 总览 | [docs/knowledge/code_map/netmanager-ext-code-map.md](docs/knowledge/code_map/netmanager-ext-code-map.md) |
| 以太网请求如何一步步执行 | [docs/knowledge/code_map/ethernet-pipeline-map.md](docs/knowledge/code_map/ethernet-pipeline-map.md) |
| VPN 建链与 uid 段路由 | [docs/knowledge/code_map/vpn-pipeline-map.md](docs/knowledge/code_map/vpn-pipeline-map.md) |
| 共享/NAT/状态机链路 | [docs/knowledge/code_map/networkshare-pipeline-map.md](docs/knowledge/code_map/networkshare-pipeline-map.md) |
| IDL 生成物与手写面判定 | [docs/knowledge/code_map/idl-generated-code-map.md](docs/knowledge/code_map/idl-generated-code-map.md) |
| 任务/现象/SA/调用方路由 | [docs/knowledge/routing/task-routing.md](docs/knowledge/routing/task-routing.md) |
| 术语速查 | [docs/knowledge/routing/vocabulary-routing.md](docs/knowledge/routing/vocabulary-routing.md) |
| 历史高危修复与案例 | [docs/knowledge/expert/historical-failures.md](docs/knowledge/expert/historical-failures.md) |
| 并发/状态机/恢复约束 | [docs/knowledge/expert/state-machine-concurrency.md](docs/knowledge/expert/state-machine-concurrency.md) |
| 权限与调用方安全 | [docs/knowledge/expert/permission-security-constraints.md](docs/knowledge/expert/permission-security-constraints.md) |
| 跨组件边界 | [docs/knowledge/expert/cross-component-boundaries.md](docs/knowledge/expert/cross-component-boundaries.md) |
| 编译与测试命令 | [docs/knowledge/verify/build-and-test.md](docs/knowledge/verify/build-and-test.md) |
| 验证循环与 Done 定义 | [docs/knowledge/verify/verification-loop.md](docs/knowledge/verify/verification-loop.md) |

## 4. 词汇触发路由

| 术语 | 含义 | 指向 |
|---|---|---|
| `FAKE_DOWNSTREAM_IFACENAME` | 主状态机无真实下行口时的占位网卡名（`networkshare_main_statemachine.cpp:31`） | [state-machine-concurrency.md](docs/knowledge/expert/state-machine-concurrency.md) |
| `netshareRequester_`/`clientRequestsBitMask_` | tracker 请求方位图（atomic，2dd7ac1） | [historical-failures.md](docs/knowledge/expert/historical-failures.md) |
| `hasOpenedVpnUid_` | 已开 VPN 的调用方 uid（`networkvpn_service.cpp:945`） | [permission-security-constraints.md](docs/knowledge/expert/permission-security-constraints.md) |
| `Prepare`/`SetUpVpn` | 三方 VPN 前置申请（:322）与建链（:576/:880 两处都要校验） | [permission-security-constraints.md](docs/knowledge/expert/permission-security-constraints.md) |
| Protect/fwmark、uid 段路由 | VPN 保护标记与分段流量（`net_vpn_impl.cpp`） | [vpn-pipeline-map.md](docs/knowledge/code_map/vpn-pipeline-map.md) |
| RA/RNDIS | 路由通告守护（f5cc878）/USB 共享网卡形态 | [networkshare-pipeline-map.md](docs/knowledge/code_map/networkshare-pipeline-map.md) |
| URSP | 切片路由策略配置（`urspconfig.cpp`） | [netmanager-ext-code-map.md](docs/knowledge/code_map/netmanager-ext-code-map.md) |
| NetSupplier | 设备网络向 base netconn 注册的载体（`dev_interface_state.cpp:161/:188`） | [ethernet-pipeline-map.md](docs/knowledge/code_map/ethernet-pipeline-map.md) |
| EDM | 企业设备管理策略（VPN :1068-1069；share `persist.edm.tethering_disallowed`） | [permission-security-constraints.md](docs/knowledge/expert/permission-security-constraints.md) |
| `NetsysController` | base 仓内核唯一通道 | [cross-component-boundaries.md](docs/knowledge/expert/cross-component-boundaries.md) |
| `SharingIfaceType`/`SharingIfaceState` | 共享类型/状态枚举（d.ts :491/:458） | [vocabulary-routing.md](docs/knowledge/routing/vocabulary-routing.md) |

## 5. 编辑前声明要求

Agent 在编辑任何文件前，**必须在回复中显式回答以下三项**（缺一不可，答不出必须先读 §3 对应文档）：

1. **本次任务类别**：API/绑定变更 / IPC 新增 / 服务链修改 / 崩溃修复 / 并发修复 / 跨仓联动 / 工具与测试 / 文档（多选可叠加，如「并发修复 + share」）。
2. **已读哪些知识文档**：列出本次已读的 `docs/knowledge/` 文档路径；至少包含任务类别对应的 §3 场景文档一篇。
3. **已知约束中哪些适用本任务**：从 §6 中点名适用条目（如「适用：内核动作必须经 base `NetsysController`；适用：tracker 为 UAF 高发区需绑定回归」）。

## 6. 约束和边界

### 6.1 禁止事项（红线）

- ⛔ 禁止 ext 代码直连内核 socket/iptables——内核动作**唯一**经 base `NetsysController`。
- ⛔ 禁止自造权限字符串与错误码：权限常量唯一出口 base 仓 `netmanager_base_permission.h`，220 段错误码先在 base 登记。
- ⛔ 禁止修改/手写覆盖 IDL 生成物（生成 stub/proxy 编译期产出不落仓；改 `.idl` 源与 BUILD 声明，手写面仅 netfirewall/networkslice）。
- ⛔ 禁止状态机迁移不对称（`EnableNat`/`DisableNat`）、非幂等；禁止锁内调 IPC/等待回调。
- ⛔ 禁止回调入口不判空（upstreammonitor 4 处判空是底线样板）；注册/注销必须成对。
- ⛔ 禁止删除/注释失败测试使测试通过；修复必须绑定可复现用例（51e8b7d 样板）。
- ⛔ 禁止改动开关默认值而不做 true/false 两态编译。
- ⛔ 禁止未经用户确认提交代码。

### 6.2 需确认规则（改动前向用户确认）

- 改红区大文件（`networkvpn_service.cpp` 2960 行、`networkshare_tracker.cpp` 1755 行、`ethernet_management.cpp` 1131 行等，全表见 [build-and-test.md](docs/knowledge/verify/build-and-test.md) §5）：确认已绑定对应历史案例回归复验。
- 改 1154/1155 SA profile（两仓双份）：确认 base 仓 `.json` 与本仓 `.xml` 同步方案。
- crash 恢复路径/SA 订阅逻辑：确认 crash 注入用例已列（5842ca2/4f6fff0/124a016 三查）。
- 依赖 HDI/DHCP/WiFi/蓝牙/USB 仓接口的适配改动：确认三方 MR 对齐（先 base 后 ext，逆序回退）。

### 6.3 架构不变量

- 内核通道唯一；错误码/权限/观测在 base；supplier 注册走 base netconn（`dev_interface_state.cpp:161/:188`）。
- proxy/stub 成对：IDL 面（9 个 `.idl`）与手写面（netfirewall/networkslice）判定见 [idl-generated-code-map.md](docs/knowledge/code_map/idl-generated-code-map.md)。
- API 基线以 `interfaces/kits/js/*.d.ts` 与 `interfaces/innerkits` 交付 SO 为准；同一能力变更 JS/C/ANI/native 四面成套同步。
- SA 服务事件恢复链固定：`OnAddSystemAbility` 必须重订阅（样板 `networkvpn_client.cpp:153/:162/:167`；`ethernet_service.cpp:86/:102`）。
- 权限校验顺序固定：权限 → 实取身份（`GetCallingUid`，禁止信任入参）→ 策略（EDM）→ 才可改状态。

### 6.4 项目特定陷阱（历史失效模式排行）

1. **并发/竞态/UAF/ABBA**（第一热点）：`networkshare_tracker.cpp`（2dd7ac1）、`ethernet_management.cpp`（30cd0bb）、`networkvpn_service.cpp`（4e24608）。
2. **空指针**：`router_advertisement_daemon.cpp`（f5cc878）、`networkshare_upstreammonitor.cpp`（ec54b35）。
3. **内存泄漏（异常分支/畸形报文）**：`mdns_packet_parser.cpp`（51e8b7d）、ethernetclient/mdnsclient Parcel 4 文件（23c57cd）。
4. **进程 crash 恢复状态错误**：热点误重开（5842ca2）、子进程残留（4f6fff0）、client 失联（124a016）。
5. **校验顺序/漏第二入口**：VPN `SetUpVpn` 两处（:576/:880）只补一处。

台账与验证要点详见 [historical-failures.md](docs/knowledge/expert/historical-failures.md)。

### 6.5 Agent 失败模式（自我检查）

- 只改 JS 面，漏 `frameworks/c`/`frameworks/ets/ani`/native 客户端 → 改接口后按 §6.3 成套核对。
- 补权限只补主入口漏重载 → 以 `SetUpVpn` 两处为鉴grep 全部同名路径。
- 恢复路径只跑正常用例 → crash 注入三查不可省。
- 忘开关两态、忘 `netfirewall_packages` 默认不编入 → 按 [build-and-test.md](docs/knowledge/verify/build-and-test.md) §4。

## 7. 构建和验证

### 7.1 编译（全部命令见 [build-and-test.md](docs/knowledge/verify/build-and-test.md) §1）

```bash
# 常用域组（gn 组实名）：ethernet_packages / share_packages / mdns_packages / vpn_packages /
# vpn_ext_packages / netfirewall_packages / wearable_distributed_net_packages / networkslice_packages / common_ext_packages
./build.sh --product-name rk3568 --build-target share_packages --ccache
# 测试聚合
./build.sh --product-name rk3568 --build-target netmanager_ext_test --ccache
```

### 7.2 改动 → 必跑测试（全表见 [build-and-test.md](docs/knowledge/verify/build-and-test.md) §3）

| 改动区域 | 必跑测试 target |
|---|---|
| 以太网 | `ethernet_manager_test`、`EthernetClientFuzzTest` |
| 共享 | `networkshare_manager_test`、`NetShareClientFuzzTest` + 并发压测 |
| VPN | `vpn_manager_test`、`vpn_manager_ext_test`、`sys_vpn_manager_test`、`VpnClientFuzzTest`、`VpnClientRequestVpnPermissionFuzzTest` |
| mDNS | `mdns_manager_test`、`mdns_protocol_test`、`MDnsClientFuzzTest` |
| 防火墙（先开开关） | `netfirewallmanager_test`、`NetFirewallClientFuzzTest` |
| 切片 | `networkslice_manager_test` |
| 穿戴 | `wearable_distributed_net_manager_test` |

### 7.3 Done 定义

六条全部满足（编译含两态 / 矩阵测试全过 / 红区绑定回归 / API 兼容未破坏四面同步 / 红线零违反 / clang-format）才算完成——逐条见 [verification-loop.md](docs/knowledge/verify/verification-loop.md) §3。

### 7.4 回退方案

- 验证不过且短期无法修复：`git revert`，不留带病代码；跨仓面按逆序回退（ext → base → 硬件仓）。
- **回退后必须把失效模式与教训回写** [historical-failures.md](docs/knowledge/expert/historical-failures.md)（hash+现象+根因+教训，只增不删）。

## 8. 文档维护

```
docs/knowledge/
├── code_map/    # 代码地图：netmanager-ext-code-map / ethernet-pipeline-map / vpn-pipeline-map / networkshare-pipeline-map / idl-generated-code-map
├── routing/     # 路由：task-routing / vocabulary-routing
├── expert/      # 专家约束：historical-failures / state-machine-concurrency / permission-security-constraints / cross-component-boundaries
└── verify/      # 验证：build-and-test / verification-loop
```

更新规则：

- **新失效模式**（新崩溃形态、revert 教训）必须回写 `expert/historical-failures.md`，并在 `routing/task-routing.md` §2 现象表加条目。
- 新增目录/模块/测试 target/SA：更新对应 `code_map/` 文档、`verify/build-and-test.md` 与本文 §1/§2。
- 新增术语：更新 `routing/vocabulary-routing.md` 与本文 §4。
- 修改编译命令/测试 target：同步 `verify/build-and-test.md`、`verification-loop.md` 与本文 §7。
- 知识文档只增不删；过期内容标注 `已废弃` 并保留原文（历史教训不可丢弃）。
