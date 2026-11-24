# netmanager_ext 构建与测试指南（build-and-test）

> 组件 `@ohos.netmanager_ext` | subsystem `communication` | 验证顺序与 Done 定义见 [verification-loop.md](verification-loop.md)
> 本表全部为**实名核验**过的 gn 目标/测试名；改动对应区域后按 §3 矩阵跑对应测试。

## 1. 编译

```bash
# 域组整体（gn 组实名，按改动域选）
./build.sh --product-name rk3568 --build-target ethernet_packages --ccache
./build.sh --product-name rk3568 --build-target share_packages --ccache
./build.sh --product-name rk3568 --build-target mdns_packages --ccache
./build.sh --product-name rk3568 --build-target vpn_packages --ccache
./build.sh --product-name rk3568 --build-target vpn_ext_packages --ccache
./build.sh --product-name rk3568 --build-target netfirewall_packages --ccache
./build.sh --product-name rk3568 --build-target wearable_distributed_net_packages --ccache
./build.sh --product-name rk3568 --build-target networkslice_packages --ccache
./build.sh --product-name rk3568 --build-target common_ext_packages --ccache
```

- 打包交付物在 `bundle.json` `service_group`：各域 packages + SA profile（`sa_profile:mdns_manager_profile` 等 4 个）+ init 配置（`services/etc/init` 下 `ethernet.cfg`/`vpnmanager.cfg`/`mdnsmanager.rc`/`UrspConfig_xml` 等）+ `frameworks/ets/ani:ani_package` + `tools/ohos-networkShare` + `vpn_dialog` hap。
- innerkits 交付 SO（`interfaces/innerkits`）：`net_tether_manager_if`、`ethernet_manager_if`、`mdns_manager_if`、`vpn_extension_module`、`net_vpn_manager_if`、`net_vpn_permission_if`、`netfirewall_manager_if`、`wearable_distributed_net_manager_if`、`networkslice_manager_if`。改 innerkit 头文件=对外兼容问题，对照 base 仓 API 兼容口径。
- version_script 符号收敛（2026-11 接入，报告见 `~/code/so_size_opt/netmanager_ext_version_script_report.md`）：7 个 SA 根 so 全屏蔽 `{local: *;}`；innerkit 用 `_Z*4OHOS*` mangled 通配；`sharing_ani` 仅导出 `ANI_Constructor`；NDK `net_ethernet_ndk`/`net_trafficfilter` 按头文件枚举/前缀通配；`net_event_report`/`vpn_extension`/`vpn_extension_module` 按消费符号收敛。napi 模块库由构建系统自动注入 napi.versionscript。**新增 so 目标必须同步接 version_script**；测试白盒调内部符号走 `*_static` testonly 静态孪生（ethernet/share/mdns/vpn/firewall/slice/wearable 均有），勿为测试在产品 ABI map 开洞。注意：netfirewall/wearable/networkslice SA 域开关默认 OFF，**编译验证必须用对应产品 device-type**（PC=`general_all_pc_standard` 编 netfirewall、手表=`general_all_watch_standard` 编 wearable）——产品的 `vendor/huawei/build/component_config/.../part_config.json` 会同时覆盖 base+ext 两侧开关（代码仓默认值被产品配置覆盖），宏（`FEATURE_NET_FIREWALL_ENABLE` 等）经 base 目标 public config 传播到 ext TU；在手机产品上手工 `--gn-args` 只开 ext 侧开关必然编译失败（base 侧宏未定义）。networkslice 商用不开启。ext 测试模块只有通过 `test:netmanager_ext_test` 聚合组（bundle.json test 入口）才会进构建图，直接传模块名会报 unknown target。

## 2. 测试实名清单

单测（`ohos_unittest`，共 10 个）：

| 域 | target |
|---|---|
| 以太网 | `ethernet_manager_test` |
| 共享 | `networkshare_manager_test` |
| VPN | `vpn_manager_test`、`vpn_manager_ext_test`、`sys_vpn_manager_test` |
| mDNS | `mdns_manager_test`、`mdns_protocol_test` |
| 防火墙 | `netfirewallmanager_test` |
| 切片 | `networkslice_manager_test` |
| 穿戴 | `wearable_distributed_net_manager_test` |

Fuzz（`ohos_fuzztest`，共 6 个）：`EthernetClientFuzzTest`、`NetShareClientFuzzTest`、`MDnsClientFuzzTest`、`VpnClientFuzzTest`、`VpnClientRequestVpnPermissionFuzzTest`、`NetFirewallClientFuzzTest`。

测试聚合组：`//foundation/communication/netmanager_ext/test:netmanager_ext_test`（一次性全跑入口）。

## 3. 改动 → 必跑矩阵

| 改动区域 | 必跑 |
|---|---|
| 以太网链路（`ethernet_management`/`dev_interface_state`） | `ethernet_manager_test`、`EthernetClientFuzzTest` |
| 共享 tracker/状态机 | `networkshare_manager_test`、`NetShareClientFuzzTest`；并发改动加速度压测（见 [../expert/state-machine-concurrency.md](../expert/state-machine-concurrency.md) §7） |
| VPN 链路/授权 | `vpn_manager_test`、`vpn_manager_ext_test`、`sys_vpn_manager_test`、`VpnClientFuzzTest`、`VpnClientRequestVpnPermissionFuzzTest` |
| mDNS 协议/解析器 | `mdns_manager_test`、`mdns_protocol_test`、`MDnsClientFuzzTest` |
| 防火墙（先开 `netmanager_ext_feature_net_firewall`） | `netfirewallmanager_test`、`NetFirewallClientFuzzTest` |
| 切片/URSP | `networkslice_manager_test` |
| 穿戴 | `wearable_distributed_net_manager_test` |
| IDL 接口重新生成 | 对应域编译 + 本域单测全跑 |
| 红区大文件（§5） | 上表全部相关项 + [../expert/historical-failures.md](../expert/historical-failures.md) 对应案例复验 |

## 4. 开关矩阵（`netmanager_ext_config.gni` declare_args 实名）

| 默认值 | 开关 |
|---|---|
| true（默认编入） | `netmanager_ext_feature_ethernet`、`netmanager_ext_feature_share`、`netmanager_ext_feature_mdns`、`netmanager_ext_feature_vpn`、`netmanager_ext_feature_vpnext`、`netmanager_ext_enable_debug`（即 `enable_netmgr_ext_debug`） |
| false（默认裁剪） | `netmanager_ext_feature_net_firewall`、`netmanager_ext_feature_sysvpn`、`netmanager_ext_feature_vpn_for_user0`、`netmanager_ext_feature_wearable_distributed_net`、`netmanager_ext_feature_networkslice`、`netmanager_ext_feature_iface_supplier_id`、`netmanager_ext_feature_ethernet_enable_disable`、`netmanager_ext_extensible_authentication`、`share_notification_enable`、`share_traffic_limit_enable`、`feature_coverage`、`fpga_mode_enable`、`read_ccm_json` |
| 外部协作 | `communication_wifi_switch_enable`、`communication_bluetooth_switch_enable`、`usb_manager_enable`、`battery_manager_switch_enable` |

规则：改任何开关默认值或依赖开关的代码，**true/false 两态都要编译**再合入；防火墙/切片默认关，对应测试需先开开关编入。

## 5. 红区大文件（改动绑定回归）

| 文件 | 行数 | 关联案例 |
|---|---|---|
| `services/vpnmanager/src/networkvpn_service.cpp` | 2960 | 4e24608、4f6fff0 |
| `services/networkslicemanager/src/urspconfig.cpp` | 2148 | — |
| `services/networksharemanager/src/networkshare_tracker.cpp` | 1755 | 2dd7ac1、5842ca2 |
| `services/netfirewallmanager/src/nettrafficfilter_redirect_manager.cpp` | 1413 | — |
| `services/networkslicemanager/src/hwnetworkslicemanager.cpp` | 1361 | — |
| `services/ethernetmanager/src/ethernet_management.cpp` | 1131 | 30cd0bb |
| `services/networkslicemanager/src/networkslicemanager.cpp` | 1185 | — |
| `services/mdnsmanager/src/protocol/mdns_protocol_impl.cpp` | 1049 | 51e8b7d |

## 6. 观测与落盘核对

- hisysevent 事件定义在 **base 仓**（domain `NETMANAGER`），ext 只上报已登记事件（见 [../expert/cross-component-boundaries.md](../expert/cross-component-boundaries.md) §2）。
- SA 启动配置落盘：`sa_profile/`（1154.xml/1155.xml/1161.json/8300.json/8301.json/8400.json）与 `services/etc/init/` 各 cfg/rc；改进程名/onDemand 时编译产物里核对两处是否同步。

## 关联知识

- 验证循环：[verification-loop.md](verification-loop.md)
- 编译开关边界：[../expert/cross-component-boundaries.md](../expert/cross-component-boundaries.md) §6

## 适用场景

- 提交前选择编译目标与测试集；
- 新增测试 target 后回填本清单（只增不删）；
- 排查"开关没开导致测试没编入"类问题。
