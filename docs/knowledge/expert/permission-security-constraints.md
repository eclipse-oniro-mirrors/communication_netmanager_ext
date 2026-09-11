# netmanager_ext 权限与调用方安全约束（permission-security-constraints）

> 组件 `@ohos.netmanager_ext` | subsystem `communication` | 术语背景见 [../routing/vocabulary-routing.md](../routing/vocabulary-routing.md)
> ext 的接口全部是"改设备网络状态"的敏感能力：**先校验，后动状态**是唯一合法顺序。

## 1. 权限常量唯一出口（跨仓铁律）

- 所有权限字符串常量（如 `CONNECTIVITY_INTERNAL`、`MANAGE_VPN`）只定义在 **base 仓** `utils/common_utils/include/netmanager_base_permission.h`，ext 仓**禁止自造**权限字符串。
- 校验封装统一走 `CheckPermission(...)` 模式；新增接口先看 base 仓是否已有对应常量，没有则先在 base 仓补，再在 ext 引用（联动红线见 [cross-component-boundaries.md](cross-component-boundaries.md)）。

## 2. share 域检查点

| 检查点 | 实位 | 规则 |
|---|---|---|
| 共享开关/查询接口 | `services/networksharemanager/src/networkshare_service.cpp:175/:187/:204/:233/:286` 等 10+ 处 `CheckPermission` | 每个 IPC 入口都要过权限，新增入口照抄该模式 |
| CLI 排查口 | `tools/ohos-networkShare` | 仅限系统调用方；且受 EDM 禁共享策略（`persist.edm.tethering_disallowed`）约束，被禁时拒绝 |
| 通知/限流开关 | `share_notification_enable`、`share_traffic_limit_enable`（默认 false） | 关闭时相应能力不出接口行为 |

## 3. VPN 域检查点（身份校验最密集区）

| 检查点 | 实位 | 规则 |
|---|---|---|
| 权限门槛 | `networkvpn_service.cpp:689`（`MANAGE_VPN`） | 系统侧 VPN 变更统一门槛 |
| `Prepare` 前置 | `:322` | 三方 VPN 必须先行 `Prepare`，授权未过不得建链 |
| 建链入口两处 | `SetUpVpn:576` 与 `:880` | ⚠️ 两个重载/路径**都要**校验；历史缺陷多出现在"补了 A 忘了 B" |
| 调用方身份 | `GetCallingUid:219`/`:752` | 拿到的 uid 必须与状态比对后使用，禁止直接信任入参 uid |
| 已开 VPN 比对 | `hasOpenedVpnUid_ = GetCallingUid`（`:945`） | 二次请求要和已记录的 uid 对齐， uid 不一致拒绝，防越权接管 |
| EDM 管控 | `:1068-1069` | 企业策略允许/禁用 VPN 的判定在改动隧道状态**之前**完成 |
| 用户授权 | `frameworks/vpn_dialog` | 三方 VPN 依赖用户在授权弹窗确认，授权结果回传后才继续 |
| fuzz 面 | `VpnClientRequestVpnPermissionFuzzTest`（test/fuzztest） | 权限申请链路有专门 fuzzer，改授权流程必跑 |

## 4. 校验顺序规范（防绕过）

合法顺序固定为，禁止调换：

1. **权限/token 校验**（§1 常量 + `CheckPermission`）；
2. **调用方身份取值与比对**（`GetCallingUid`/pid，与 `hasOpenedVpnUid_`、`setVpnPidMap_` 等状态交叉验证）；
3. **策略判定**（EDM 等 MDM 策略）；
4. 最后才是改状态/起进程/下发内核。

- ❌ 常见违规：先改状态后补校验（竞态窗口内已完成动作）；用客户端传入的 uid/pid 字段代替 `GetCallingUid`。
- ⚠️ 新增 EDM 类策略时，同时核对 share 域 `persist.edm.tethering_disallowed` 的既有模式，保持策略判定位置一致。

## 5. 自检清单

1. 新 IPC 入口是否逐个 `CheckPermission`？（share 域 10+ 处是密度基准）
2. 有无"补了主入口、漏了重载/第二入口"？（VPN `SetUpVpn` 两处为鉴）
3. 身份是 `GetCallingUid` 实取还是信任入参？
4. EDM/策略判定是否在状态变更之前？
5. 权限字符串是否引用 base 仓常量，而不是本地字面量？
6. 改动后对应 fuzzer（如 `VpnClientRequestVpnPermissionFuzzTest`）是否已跑？

## 关联知识

- 并发侧状态竞态：[state-machine-concurrency.md](state-machine-concurrency.md)
- 跨仓权限常量联动：[cross-component-boundaries.md](cross-component-boundaries.md)
- IPC 层通用安全规则（base 仓口径）：netmanager_base `docs/knowledge/expert/ipc-security-constraints.md`

## 适用场景

- 新增任何 IPC 接口（service/stub 两端）；
- 改动 VPN 授权、共享开关、EDM 策略路径；
- 安全评审/渗透走查 ext 敏感接口时作检查表。
