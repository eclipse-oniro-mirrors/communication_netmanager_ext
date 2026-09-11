# netmanager_ext 跨组件边界约束（cross-component-boundaries）

> 组件 `@ohos.netmanager_ext` | subsystem `communication` | 全图见 [../code_map/netmanager-ext-code-map.md](../code_map/netmanager-ext-code-map.md)
> ext 单仓改不动的边界全在本文：**内核在 base 仓、HDI/DHCP/硬件栈在别的仓、错误码/权限/观测也在 base 仓**。

## 1. 边界总图

```
 JS/NDK/ArkTS 应用 ──┐
                    ▼
  ext 绑定层：interfaces/kits/js(*.d.ts) + frameworks/{js,c,ets,native,vpn_dialog}
                    ▼ IPC(IDL 生成面 │ 手写面)
  ext 服务层：SA 1154/1155/1157/1161/8300/8301/8400
        │ eth → libethernet_proxy（HDI）→ 内核网卡
        │ dhcp → DHCP 扩展服务（dhcp 仓）
        │ share → WiFi/蓝牙/USB 事件回调（各硬件仓）
        │ 内核网络动作（NAT/标记/规则）──唯一通道──► base 仓 NetsysController ─► netsys SA 1158 ─► 内核
        └ supplier/网络信息 ──► base 仓 netconn（NetConnClient）
```

任何 ext 代码直接敲内核 socket/iptables = 越界违规。

## 2. 对 netmanager_base 的边界（最常用）

| 事项 | 规则 |
|---|---|
| 内核操作 | 一律经 base `NetsysController`（NAT/转发须在 base 侧能力齐备后调用）；ext 无直连内核路径 |
| 错误码 | 220 段错误码常量定义在 base 仓错误码头文件；ext 新错误码先去 base 登记，禁止本地造码 |
| 权限常量 | 唯一出口 base 仓 `netmanager_base_permission.h`（详见 [permission-security-constraints.md](permission-security-constraints.md) §1） |
| 观测事件 | hisyseevent 定义集中在 **base 仓**（domain `NETMANAGER`）；ext 侧只上报 base 仓已登记事件 |
| 网络注册 | 以太网口通过 `dev_interface_state.cpp` 的 `RemoteRegisterNetSupplier:161`/`RemoteUpdateNetLinkInfo:188` 走 base netconn supplier 体系 |
| SA profile | 1154/1155 **base 有 `.json`、ext 有 `.xml` 双份**；1157 只在 base。改注册行为两仓同步（见 [../routing/task-routing.md](../routing/task-routing.md) §3） |
| 启动顺序 | `ethernet_service.cpp:86` 等 base SA 1151 就绪后才 `InitManagement:102`——ext 服务依赖 base 服务拉起顺序 |

## 3. 对 HDI / DHCP / 硬件栈的边界

- **以太网 HDI**：`libethernet_proxy_1.0`（HDI 代理）；网卡在离线/mtu 等读 HDI 事件，链路配置经 base 下内核。
- **DHCP**：子状态机 DHCP 服务端动作经 `dhcp_c_api` 到 DHCP 扩展服务（独立仓），ext 不自实现 DHCP 协议（`networkshare_sub_statemachine.cpp:954`/`:977`）。
- **硬件栈联动编译**：`netmanager_ext_config.gni` 定义的外部开关决定能力面——`communication_wifi_switch_enable`、`communication_bluetooth_switch_enable`、`usb_manager_enable`、`battery_manager_switch_enable`；关闭对应硬件仓时 ext 对应路径（热点/BT PAN/USB 共享/流量限流）同步裁剪。⚠️ 改共享/限流功能时四个开关的 true/false 两态都要编译验证。
- **热点能力**：share 子状态机 WIFI 路径复用 WiFi 热点（`wifi_hotspot.h:35`）；WiFi 仓接口变更会穿透到 `networkshare_sub_statemachine.cpp`。

## 4. IDL 生成面 vs 手写面（改 IPC 前先判定）

- 9 个 `.idl` 源（`IEthernetService.idl`、`IMdnsService.idl`+3 回调、`INetworkShareService.idl`、`INetworkVpnService.idl`+`IVpnEventCallback.idl`、`IWearableDistributedNet.idl`）编译期生成 stub/proxy，**不落仓**；
- netfirewall / networkslice 为手写 proxy/stub 面；
- 判定方法与新增命令最小动作集见 [../code_map/idl-generated-code-map.md](../code_map/idl-generated-code-map.md)。⛔ 禁止给 IDL 生成路径手写同名 stub 文件覆盖生成物。

## 5. 多语言绑定同步红线

同一能力变更需成套改：

| 面 | 位置 |
|---|---|
| API 基线（以 d.ts 为准） | `interfaces/kits/js/`（`@ohos.net.ethernet.d.ts`、`@ohos.net.sharing.d.ts`、`@ohos.net.mdns.d.ts`、`@ohos.net.eap.d.ts` 等） |
| JS 实现 | `frameworks/js/napi/{ethernet,sharing,vpn,vpnext,mdns,netfirewall,extensionability,extensioncontext}` |
| native 客户端 | `frameworks/native/{ethernetclient,netshareclient,netvpnclient,mdnsclient,netfirewallclient,networksliceclient,vpnextension,wearabledistributednetclient}` |
| C 绑定 | `frameworks/c` |
| ArkTS/Rust ANI | `frameworks/ets/ani`（打包目标 `ani_package`，见 bundle.json） |

漏改任一面＝调用方行为不一致。改接口签名时逐面核对，并对照 base 仓 API 兼容口径。

## 6. 配置与构建产物边界

- 编译开关全部集中在 `netmanager_ext_config.gni` `declare_args`（实名表见 [../verify/build-and-test.md](../verify/build-and-test.md) §4），禁止在模块 BUILD.gn 里重复 declare。
- 切片配置：`urspconfig.cpp` 解析 URSP XML；新增 URSP 字段须同步配置文件与解析器两侧。
- 安全编译：公共 cflags 含 `-D_FORTIFY_SOURCE=2`、cfi blocklist 有登记文件——新增分支敏感代码时确认未被裁掉。

## 7. 协作改动流程（跨仓 MR）

1. 先 base 后 ext：权限常量/错误码/内核能力在 base 仓 MR 合入后，ext 再引用；
2. 涉及 HDI/DHCP/硬件仓接口变更：三方仓库 MR 对齐后同步改 ext 适配层；
3. 开关默认值变更：必须列出受影响 target 清单并两态编译（§3）；
4. 回退跨仓 MR 时按逆序回退，回退教训回写 [historical-failures.md](historical-failures.md)。

## 关联知识

- IPC 安全细则（stub/proxy/Parcel）：base 仓 `docs/knowledge/expert/ipc-security-constraints.md`
- IDL 判定：[../code_map/idl-generated-code-map.md](../code_map/idl-generated-code-map.md)
- 开关/编译验证：[../verify/build-and-test.md](../verify/build-and-test.md)

## 适用场景

- 改动需要动内核、动 base 仓能力、动 HDI/DHCP/硬件接口时；
- 新增/修改编译开关、裁剪能力面时；
- 接口变更需要多语言绑定成套同步时；
- 排查"base 己变了导致 ext 编译/行为异常"类问题。
