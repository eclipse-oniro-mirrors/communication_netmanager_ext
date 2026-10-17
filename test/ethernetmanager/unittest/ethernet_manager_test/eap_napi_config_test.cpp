/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include <cstring>

#include "napi_mock.h"

/* 被测源文件直接纳入本编译单元：
 * 1) 文件内 static 函数/全局变量（GetEapErrMsg、g_stateCallback 等）可直接驱动；
 * 2) napi_* 符号由 napi_mock.cpp 的强定义覆盖（可执行文件定义在链接期胜出）。
 * 说明：native 单测中不存在真实 JS 虚拟机，本用例通过 napi 环境模拟驱动
 * eap_napi_config.cpp 的解析/序列化/异常/Promise/线程投递全部逻辑；
 * 经 EthernetClient 的跨进程路径在 DT 真机与无服务环境下分别返回
 * 201（权限拒绝）/33200009（服务不可达），用例对两种确定性结果均做校验。 */
#include "eap_napi_config.cpp"

namespace OHOS {
namespace NetManagerStandard {

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

using namespace testing::ext;

namespace {
constexpr int32_t TEST_CODE_PERMISSION_DENIED = 201;
constexpr int32_t TEST_CODE_NETMANAGER_STOP = 33200009;
constexpr int32_t TEST_CODE_INTERNAL = 33200099;

bool IsEnvDependentClientCode(int32_t code)
{
    return code == TEST_CODE_PERMISSION_DENIED || code == TEST_CODE_NETMANAGER_STOP;
}
} // namespace

class EthEapNapiConfigTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override
    {
        env_ = NapiMock::SharedEnv();
        NapiMock::ResetEnvRecords(env_);
        ClearGlobalState();
    }
    void TearDown() override
    {
        ClearGlobalState();
        NapiMock::ResetEnvRecords(env_);
    }

    static void ClearGlobalState()
    {
        for (auto &ref : g_stateChangeCbRefs) {
            napi_delete_reference(nullptr, ref);
        }
        g_stateChangeCbRefs.clear();
        g_stateCallback = nullptr;
    }

    napi_env env_ = nullptr;
};

/**
 * @tc.name  : GetEapErrMsgAllCodesTest001
 * @tc.number: EthEapNapiConfigTest_001
 * @tc.desc  : 全部错误码的消息文案与 d.ts @throws 文档一致，未知码回落内部错误
 */
HWTEST_F(EthEapNapiConfigTest, GetEapErrMsgAllCodesTest001, TestSize.Level1)
{
    EXPECT_STREQ(GetEapErrMsg(EAP_ERRCODE_PARAMETER_ERROR), "Parameter error");
    EXPECT_STREQ(GetEapErrMsg(EAP_ERRCODE_PERMISSION_DENIED), "Permission denied");
    EXPECT_STREQ(GetEapErrMsg(EAP_ERRCODE_INVALID_NETID), "Invalid netId");
    EXPECT_STREQ(GetEapErrMsg(EAP_ERRCODE_LOGOFF_FAIL), "Log off fail");
    EXPECT_STREQ(GetEapErrMsg(EAP_ERRCODE_INVALID_PROFILE), "Invalid profile");
    EXPECT_STREQ(GetEapErrMsg(EAP_ERRCODE_NETMANAGER_STOP), "Netmanager stop");
    EXPECT_STREQ(GetEapErrMsg(EAP_ERRCODE_INVALID_ETH_STATE), "Invalid eth state");
    EXPECT_STREQ(GetEapErrMsg(EAP_ERRCODE_ENTERPRISE_OCCUPIED), "Enterprise app occupied");
    EXPECT_STREQ(GetEapErrMsg(EAP_ERRCODE_CONFIG_LOCKED), "Configuration locked");
    EXPECT_STREQ(GetEapErrMsg(EAP_ERRCODE_CERT_PROCESS_FAILED), "Certificate process failed");
    EXPECT_STREQ(GetEapErrMsg(NETMANAGER_EXT_ERR_FILE_NOT_FOUND), "Config file not found");
    EXPECT_STREQ(GetEapErrMsg(TEST_CODE_INTERNAL), "Internal error");
    EXPECT_STREQ(GetEapErrMsg(999999), "Internal error");
}

/**
 * @tc.name  : ParseEthEapProfileFullFieldsTest002
 * @tc.number: EthEapNapiConfigTest_002
 * @tc.desc  : 从 JS 对象完整解析全部 profile 字段（含 Uint8Array 证书内容与 SecureData 口令）
 */
// 表驱动构造完整字段 profile 对象：数字/字符串两张属性表，避免测试函数内超长逐字段构造链
napi_value MakeFullProfileObj(napi_env env)
{
    napi_value obj = NapiMock::MakeObject(env);
    const std::vector<std::pair<std::string, int32_t>> numProps = {
        {"eapMethod", 3}, // EAP_TTLS
        {"phase2Method", 1},
        {"eapSubId", 1},
    };
    for (const auto &prop : numProps) {
        NapiMock::SetProp(env, obj, prop.first, NapiMock::MakeNumber(env, prop.second));
    }
    const std::vector<std::pair<std::string, std::string>> strProps = {
        {"identity", "napi-dt-identity"},
        {"anonymousIdentity", "napi-dt-anon"},
        {"password", "NapiPass@123"},
        {"caCertAliases", "napi_ca_alias"},
        {"caPath", "/data/ca.pem"},
        {"clientCertAliases", "oh:napi_client"},
        {"certPassword", "CertPass@456"},
        {"altSubjectMatch", "DNS:dt.com"},
        {"domainSuffixMatch", "dt.com"},
        {"realm", "dt.realm"},
        {"plmn", "46000"},
    };
    for (const auto &prop : strProps) {
        NapiMock::SetProp(env, obj, prop.first, NapiMock::MakeString(env, prop.second));
    }
    NapiMock::SetProp(env, obj, "certEntry", NapiMock::MakeUint8Array(env, {0x30, 0x82, 0x00, 0xFF, 0x00}));
    return obj;
}

HWTEST_F(EthEapNapiConfigTest, ParseEthEapProfileFullFieldsTest002, TestSize.Level1)
{
    napi_value profileObj = MakeFullProfileObj(env_);

    EthEapProfile profile;
    ParseEthEapProfileFromJs(env_, profileObj, profile);
    EXPECT_EQ(profile.eapMethod, EapMethod::EAP_TTLS);
    EXPECT_EQ(profile.phase2Method, static_cast<Phase2Method>(1));
    EXPECT_EQ(profile.identity, "napi-dt-identity");
    EXPECT_EQ(profile.anonymousIdentity, "napi-dt-anon");
    EXPECT_STREQ(profile.password.c_str(), "NapiPass@123");
    EXPECT_EQ(profile.caCertAliases, "napi_ca_alias");
    EXPECT_EQ(profile.caPath, "/data/ca.pem");
    EXPECT_EQ(profile.clientCertAliases, "oh:napi_client");
    EXPECT_EQ(profile.certEntry, (std::vector<uint8_t>{0x30, 0x82, 0x00, 0xFF, 0x00}));
    EXPECT_STREQ(profile.certPassword.c_str(), "CertPass@456");
    EXPECT_EQ(profile.altSubjectMatch, "DNS:dt.com");
    EXPECT_EQ(profile.domainSuffixMatch, "dt.com");
    EXPECT_EQ(profile.realm, "dt.realm");
    EXPECT_EQ(profile.plmn, "46000");
    EXPECT_EQ(profile.eapSubId, 1);
}

/**
 * @tc.name  : ParseEthEapProfileDefaultsTest003
 * @tc.number: EthEapNapiConfigTest_003
 * @tc.desc  : 空对象解析后全部字段保持默认值（缺属性视为未提供）
 */
HWTEST_F(EthEapNapiConfigTest, ParseEthEapProfileDefaultsTest003, TestSize.Level1)
{
    napi_value profileObj = NapiMock::MakeObject(env_);
    EthEapProfile profile;
    ParseEthEapProfileFromJs(env_, profileObj, profile);
    EXPECT_EQ(profile.eapMethod, EapMethod::EAP_NONE);
    EXPECT_EQ(profile.phase2Method, Phase2Method::PHASE2_NONE);
    EXPECT_TRUE(profile.identity.empty());
    EXPECT_TRUE(profile.password.empty());
    EXPECT_TRUE(profile.certEntry.empty());
    EXPECT_TRUE(profile.caCertAliases.empty());
    EXPECT_EQ(profile.eapSubId, 0);
}

/**
 * @tc.name  : ParseEthEapProfileNonUint8CertEntryTest004
 * @tc.number: EthEapNapiConfigTest_004
 * @tc.desc  : certEntry 非 Uint8Array（普通对象）与非 uint8 类型 TypedArray 均被忽略
 */
HWTEST_F(EthEapNapiConfigTest, ParseEthEapProfileNonUint8CertEntryTest004, TestSize.Level1)
{
    // 普通对象：非 typedarray
    napi_value plainObj = NapiMock::MakeObject(env_);
    napi_value profileObj1 = NapiMock::MakeObject(env_);
    NapiMock::SetProp(env_, profileObj1, "certEntry", plainObj);
    EthEapProfile profile1;
    ParseEthEapProfileFromJs(env_, profileObj1, profile1);
    EXPECT_TRUE(profile1.certEntry.empty());

    // int16 TypedArray：类型不符
    napi_value int16Array = NapiMock::MakeInt16Array(env_, 2);
    ASSERT_NE(int16Array, nullptr);
    napi_value profileObj2 = NapiMock::MakeObject(env_);
    NapiMock::SetProp(env_, profileObj2, "certEntry", int16Array);
    EthEapProfile profile2;
    ParseEthEapProfileFromJs(env_, profileObj2, profile2);
    EXPECT_TRUE(profile2.certEntry.empty());
}

/**
 * @tc.name  : CreateEthEapConfigJsTest005
 * @tc.number: EthEapNapiConfigTest_005
 * @tc.desc  : 配置序列化为 JS 对象：敏感字段（密码/口令/证书内容）不回读，certEntry 为空 Uint8Array
 */
HWTEST_F(EthEapNapiConfigTest, CreateEthEapConfigJsTest005, TestSize.Level1)
{
    EthEapConfig config;
    config.profile.eapMethod = EapMethod::EAP_PEAP;
    config.profile.phase2Method = Phase2Method::PHASE2_MSCHAPV2;
    config.profile.identity = "napi-dt-identity";
    config.profile.password.assign("ShouldNotLeak@1");
    config.profile.certPassword.assign("ShouldNotLeak@2");
    config.profile.certEntry = {0x01, 0x02};
    config.profile.caCertAliases = "napi_ca";
    config.profile.realm = "dt.realm";
    config.enabled = true;
    config.autoAuth = true;
    config.locked = true;

    napi_value jsObj = CreateEthEapConfigJs(env_, config);
    ASSERT_NE(jsObj, nullptr);
    EXPECT_EQ(jsObj->type, napi_object);
    // 顶层三开关
    EXPECT_EQ(NapiMock::GetProp(env_, jsObj, "enabled")->boolVal, true);
    EXPECT_EQ(NapiMock::GetProp(env_, jsObj, "autoAuth")->boolVal, true);
    EXPECT_EQ(NapiMock::GetProp(env_, jsObj, "locked")->boolVal, true);
    // profile 子对象
    napi_value__ *profileObj = NapiMock::GetProp(env_, jsObj, "profile");
    ASSERT_NE(profileObj, nullptr);
    EXPECT_EQ(profileObj->type, napi_object);
    EXPECT_EQ(NapiMock::GetProp(env_, profileObj, "eapMethod")->intVal, static_cast<int32_t>(EapMethod::EAP_PEAP));
    EXPECT_EQ(NapiMock::GetProp(env_, profileObj, "phase2Method")->intVal,
              static_cast<int32_t>(Phase2Method::PHASE2_MSCHAPV2));
    EXPECT_EQ(NapiMock::GetProp(env_, profileObj, "identity")->strVal, "napi-dt-identity");
    EXPECT_EQ(NapiMock::GetProp(env_, profileObj, "caCertAliases")->strVal, "napi_ca");
    EXPECT_EQ(NapiMock::GetProp(env_, profileObj, "realm")->strVal, "dt.realm");
    // 敏感字段：恒为空
    EXPECT_EQ(NapiMock::GetProp(env_, profileObj, "password")->strVal, "");
    EXPECT_EQ(NapiMock::GetProp(env_, profileObj, "certPassword")->strVal, "");
    napi_value__ *certEntry = NapiMock::GetProp(env_, profileObj, "certEntry");
    ASSERT_NE(certEntry, nullptr);
    EXPECT_TRUE(certEntry->isTypedArray);
    EXPECT_EQ(certEntry->taLength, 0u);
}

/**
 * @tc.name  : CreateEthEapStateInfoJsTest006
 * @tc.number: EthEapNapiConfigTest_006
 * @tc.desc  : 状态信息序列化为 JS 对象 {state, retryCount, message}
 */
HWTEST_F(EthEapNapiConfigTest, CreateEthEapStateInfoJsTest006, TestSize.Level1)
{
    EthEapStateInfo stateInfo;
    stateInfo.state = EthEapState::STATE_RETRYING;
    stateInfo.retryCount = 2;
    stateInfo.message = "napi-dt-message";
    napi_value jsObj = CreateEthEapStateInfoJs(env_, stateInfo);
    ASSERT_NE(jsObj, nullptr);
    EXPECT_EQ(jsObj->type, napi_object);
    EXPECT_EQ(NapiMock::GetProp(env_, jsObj, "state")->intVal, static_cast<int32_t>(EthEapState::STATE_RETRYING));
    EXPECT_EQ(NapiMock::GetProp(env_, jsObj, "retryCount")->intVal, 2);
    EXPECT_EQ(NapiMock::GetProp(env_, jsObj, "message")->strVal, "napi-dt-message");
}

/**
 * @tc.name  : BusinessErrorAndPromiseHelpersTest007
 * @tc.number: EthEapNapiConfigTest_007
 * @tc.desc  : BusinessError 构造/抛出与 Promise 立即兑现/立即拒绝辅助函数
 */
HWTEST_F(EthEapNapiConfigTest, BusinessErrorAndPromiseHelpersTest007, TestSize.Level1)
{
    // 抛出 BusinessError：code/message 属性与文案一致
    ThrowBusinessError(env_, EAP_ERRCODE_CONFIG_LOCKED);
    ASSERT_TRUE(NapiMock::HasPendingException(env_));
    EXPECT_EQ(NapiMock::PendingExceptionCode(env_), EAP_ERRCODE_CONFIG_LOCKED);
    EXPECT_EQ(NapiMock::GetProp(env_, env_->pendingException, "message")->strVal, "Configuration locked");

    NapiMock::ResetEnvRecords(env_);
    // 立即兑现的 Promise
    napi_value resolvedPromise = CreateResolvedPromise(env_);
    ASSERT_NE(resolvedPromise, nullptr);
    EXPECT_EQ(NapiMock::ResolvedCount(env_), 1u);
    EXPECT_EQ(NapiMock::RejectedCount(env_), 0u);

    // 立即拒绝的 Promise：拒绝值为 BusinessError（code/message）
    napi_value rejectedPromise = CreateRejectedPromise(env_, EAP_ERRCODE_PARAMETER_ERROR);
    ASSERT_NE(rejectedPromise, nullptr);
    EXPECT_EQ(NapiMock::RejectedCount(env_), 1u);
    EXPECT_EQ(NapiMock::ResolvedCount(env_), 1u);
    napi_value__ *errObj = NapiMock::LastRejectedValue(env_);
    ASSERT_NE(errObj, nullptr);
    EXPECT_EQ(NapiMock::GetProp(env_, errObj, "code")->intVal, EAP_ERRCODE_PARAMETER_ERROR);
    EXPECT_EQ(NapiMock::GetProp(env_, errObj, "message")->strVal, "Parameter error");
}

/**
 * @tc.name  : SetEthEapConfigParamErrorTest008
 * @tc.number: EthEapNapiConfigTest_008
 * @tc.desc  : setEthEapConfig 参数校验：无参/非对象/缺 profile 子对象均抛出 401
 */
HWTEST_F(EthEapNapiConfigTest, SetEthEapConfigParamErrorTest008, TestSize.Level1)
{
    // 无参数
    napi_callback_info cbInfoNoArg = NapiMock::MakeCbInfo(env_, {});
    ASSERT_NE(cbInfoNoArg, nullptr);
    SetEthEapConfig(env_, cbInfoNoArg);
    EXPECT_EQ(NapiMock::PendingExceptionCode(env_), EAP_ERRCODE_PARAMETER_ERROR);

    // 非对象参数
    NapiMock::ResetEnvRecords(env_);
    napi_callback_info cbInfoStr = NapiMock::MakeCbInfo(env_, {NapiMock::MakeString(env_, "not-object")});
    SetEthEapConfig(env_, cbInfoStr);
    EXPECT_EQ(NapiMock::PendingExceptionCode(env_), EAP_ERRCODE_PARAMETER_ERROR);

    // 缺 profile 子对象
    NapiMock::ResetEnvRecords(env_);
    napi_callback_info cbInfoNoProfile = NapiMock::MakeCbInfo(env_, {NapiMock::MakeObject(env_)});
    SetEthEapConfig(env_, cbInfoNoProfile);
    EXPECT_EQ(NapiMock::PendingExceptionCode(env_), EAP_ERRCODE_PARAMETER_ERROR);
}

/**
 * @tc.name  : SetEthEapConfigValidTest009
 * @tc.number: EthEapNapiConfigTest_009
 * @tc.desc  : setEthEapConfig 合法参数：完整解析后经客户端透传（无权限/无服务环境抛出对应业务码）
 */
HWTEST_F(EthEapNapiConfigTest, SetEthEapConfigValidTest009, TestSize.Level1)
{
    napi_value profileObj = NapiMock::MakeObject(env_);
    NapiMock::SetProp(env_, profileObj, "eapMethod", NapiMock::MakeNumber(env_, 2));
    NapiMock::SetProp(env_, profileObj, "identity", NapiMock::MakeString(env_, "napi-dt"));
    NapiMock::SetProp(env_, profileObj, "password", NapiMock::MakeString(env_, "NapiPass@123"));
    napi_value paramObj = NapiMock::MakeObject(env_);
    NapiMock::SetProp(env_, paramObj, "profile", profileObj);
    NapiMock::SetProp(env_, paramObj, "enabled", NapiMock::MakeBoolean(env_, true));
    NapiMock::SetProp(env_, paramObj, "autoAuth", NapiMock::MakeBoolean(env_, false));
    NapiMock::SetProp(env_, paramObj, "locked", NapiMock::MakeBoolean(env_, false));

    napi_callback_info cbInfo = NapiMock::MakeCbInfo(env_, {paramObj});
    ASSERT_NE(cbInfo, nullptr);
    napi_value result = SetEthEapConfig(env_, cbInfo);
    // 单测进程无 ENTERPRISE_MANAGE_EAP 权限：真机服务返回 201；
    // 服务不可达环境返回 33200009。两者均为合法业务失败（同步抛出）
    EXPECT_TRUE(NapiMock::HasPendingException(env_));
    EXPECT_TRUE(IsEnvDependentClientCode(NapiMock::PendingExceptionCode(env_)));
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : GetEthEapConfigPromiseTest010
 * @tc.number: EthEapNapiConfigTest_010
 * @tc.desc  : getEthEapConfig 返回 Promise，失败路径恰好拒绝一次（环境相关业务码）
 */
HWTEST_F(EthEapNapiConfigTest, GetEthEapConfigPromiseTest010, TestSize.Level1)
{
    napi_callback_info cbInfo = NapiMock::MakeCbInfo(env_, {});
    ASSERT_NE(cbInfo, nullptr);
    napi_value promise = GetEthEapConfig(env_, cbInfo);
    ASSERT_NE(promise, nullptr);
    EXPECT_EQ(promise->type, napi_object);
    // 恰好兑现一次：拒绝（201/33200009）或成功 resolve 配置对象
    EXPECT_EQ(NapiMock::ResolvedCount(env_) + NapiMock::RejectedCount(env_), 1u);
    if (NapiMock::RejectedCount(env_) == 1u) {
        napi_value__ *errObj = NapiMock::LastRejectedValue(env_);
        ASSERT_NE(errObj, nullptr);
        EXPECT_TRUE(IsEnvDependentClientCode(NapiMock::GetProp(env_, errObj, "code")->intVal));
    }
}

/**
 * @tc.name  : DeleteEthEapConfigPromiseTest011
 * @tc.number: EthEapNapiConfigTest_011
 * @tc.desc  : deleteEthEapConfig 返回 Promise 并恰好兑现一次
 */
HWTEST_F(EthEapNapiConfigTest, DeleteEthEapConfigPromiseTest011, TestSize.Level1)
{
    napi_callback_info cbInfo = NapiMock::MakeCbInfo(env_, {});
    ASSERT_NE(cbInfo, nullptr);
    napi_value promise = DeleteEthEapConfig(env_, cbInfo);
    ASSERT_NE(promise, nullptr);
    EXPECT_EQ(NapiMock::ResolvedCount(env_) + NapiMock::RejectedCount(env_), 1u);
    if (NapiMock::RejectedCount(env_) == 1u) {
        napi_value__ *errObj = NapiMock::LastRejectedValue(env_);
        ASSERT_NE(errObj, nullptr);
        EXPECT_TRUE(IsEnvDependentClientCode(NapiMock::GetProp(env_, errObj, "code")->intVal));
    }
}

/**
 * @tc.name  : GetEthEapStateInfoSyncTest012
 * @tc.number: EthEapNapiConfigTest_012
 * @tc.desc  : getEthEapStateInfo 同步接口：失败抛出业务码异常（无服务/无权限环境）
 */
HWTEST_F(EthEapNapiConfigTest, GetEthEapStateInfoSyncTest012, TestSize.Level1)
{
    napi_callback_info cbInfo = NapiMock::MakeCbInfo(env_, {});
    ASSERT_NE(cbInfo, nullptr);
    napi_value result = GetEthEapStateInfo(env_, cbInfo);
    // 失败路径：抛异常并返回 undefined；成功路径（设备放行）返回状态对象
    if (NapiMock::HasPendingException(env_)) {
        EXPECT_TRUE(IsEnvDependentClientCode(NapiMock::PendingExceptionCode(env_)));
        EXPECT_EQ(result->type, napi_undefined);
    } else {
        EXPECT_EQ(result->type, napi_object);
    }
}

/**
 * @tc.name  : OnStateChangeParamErrorTest013
 * @tc.number: EthEapNapiConfigTest_013
 * @tc.desc  : onStateChange 参数校验：无参/非函数抛出 401
 */
HWTEST_F(EthEapNapiConfigTest, OnStateChangeParamErrorTest013, TestSize.Level1)
{
    napi_callback_info cbInfoNoArg = NapiMock::MakeCbInfo(env_, {});
    ASSERT_NE(cbInfoNoArg, nullptr);
    OnStateChange(env_, cbInfoNoArg);
    EXPECT_EQ(NapiMock::PendingExceptionCode(env_), EAP_ERRCODE_PARAMETER_ERROR);

    NapiMock::ResetEnvRecords(env_);
    napi_callback_info cbInfoStr = NapiMock::MakeCbInfo(env_, {NapiMock::MakeString(env_, "not-fn")});
    OnStateChange(env_, cbInfoStr);
    EXPECT_EQ(NapiMock::PendingExceptionCode(env_), EAP_ERRCODE_PARAMETER_ERROR);
}

/**
 * @tc.name  : OnStateChangeRegisterFailTest014
 * @tc.number: EthEapNapiConfigTest_014
 * @tc.desc  : 服务端回调注册失败（无权限/无服务）时 onStateChange 抛出内部错误且不入列表
 */
HWTEST_F(EthEapNapiConfigTest, OnStateChangeRegisterFailTest014, TestSize.Level1)
{
    // 确保走首次创建路径：向真实服务注册必然失败（201/33200009）
    g_stateCallback = nullptr;
    g_stateChangeTsf = nullptr;
    napi_callback_info cbInfo = NapiMock::MakeCbInfo(env_, {NapiMock::MakeFunction(env_)});
    ASSERT_NE(cbInfo, nullptr);
    OnStateChange(env_, cbInfo);
    EXPECT_EQ(NapiMock::PendingExceptionCode(env_), EAP_ERRCODE_INTERNAL_ERROR);
    EXPECT_TRUE(g_stateChangeCbRefs.empty());
    // 投递通道已创建、失败点在服务注册：注册失败后全局回调对象被清理，保持未初始化状态
    EXPECT_NE(g_stateChangeTsf, nullptr);
}

/**
 * @tc.name  : OnStateChangeRegisteredTest015
 * @tc.number: EthEapNapiConfigTest_015
 * @tc.desc  : 回调对象已就绪时 onStateChange 成功：创建引用并入列表（可重复注册多个）
 */
HWTEST_F(EthEapNapiConfigTest, OnStateChangeRegisteredTest015, TestSize.Level1)
{
    // 预置回调对象与投递通道（覆盖成功路径，不依赖服务端）
    ASSERT_TRUE(CreateStateChangeTsf(env_));
    g_stateCallback = sptr<EthEapStateNapiCallback>::MakeSptr();
    ASSERT_NE(g_stateCallback, nullptr);

    napi_value fn1 = NapiMock::MakeFunction(env_);
    napi_value fn2 = NapiMock::MakeFunction(env_);
    napi_callback_info cbInfo1 = NapiMock::MakeCbInfo(env_, {fn1});
    napi_callback_info cbInfo2 = NapiMock::MakeCbInfo(env_, {fn2});
    ASSERT_NE(cbInfo1, nullptr);
    ASSERT_NE(cbInfo2, nullptr);

    napi_value result1 = OnStateChange(env_, cbInfo1);
    EXPECT_FALSE(NapiMock::HasPendingException(env_));
    EXPECT_EQ(g_stateChangeCbRefs.size(), 1u);
    EXPECT_NE(result1, nullptr);
    OnStateChange(env_, cbInfo2);
    EXPECT_FALSE(NapiMock::HasPendingException(env_));
    EXPECT_EQ(g_stateChangeCbRefs.size(), 2u);
}

/**
 * @tc.name  : OffStateChangeParamErrorTest016
 * @tc.number: EthEapNapiConfigTest_016
 * @tc.desc  : offStateChange 参数校验：非函数且非 undefined 抛出 401；回调未初始化时静默返回
 */
HWTEST_F(EthEapNapiConfigTest, OffStateChangeParamErrorTest016, TestSize.Level1)
{
    // 非法参数类型
    napi_callback_info cbInfoNum = NapiMock::MakeCbInfo(env_, {NapiMock::MakeNumber(env_, 5)});
    ASSERT_NE(cbInfoNum, nullptr);
    OffStateChange(env_, cbInfoNum);
    EXPECT_EQ(NapiMock::PendingExceptionCode(env_), EAP_ERRCODE_PARAMETER_ERROR);

    // 合法 undefined + 回调未初始化：静默成功
    NapiMock::ResetEnvRecords(env_);
    g_stateCallback = nullptr;
    napi_callback_info cbInfoUndef = NapiMock::MakeCbInfo(env_, {NapiMock::MakeUndefined(env_)});
    ASSERT_NE(cbInfoUndef, nullptr);
    napi_value result = OffStateChange(env_, cbInfoUndef);
    EXPECT_FALSE(NapiMock::HasPendingException(env_));
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : OffStateChangeFunctionMatchTest017
 * @tc.number: EthEapNapiConfigTest_017
 * @tc.desc  : offStateChange 指定回调：同一 JS 函数对象精确匹配移除，未匹配不影响列表
 */
HWTEST_F(EthEapNapiConfigTest, OffStateChangeFunctionMatchTest017, TestSize.Level1)
{
    // 预置已注册状态
    ASSERT_TRUE(CreateStateChangeTsf(env_));
    g_stateCallback = sptr<EthEapStateNapiCallback>::MakeSptr();
    napi_value fn1 = NapiMock::MakeFunction(env_);
    napi_value fn2 = NapiMock::MakeFunction(env_);
    OnStateChange(env_, NapiMock::MakeCbInfo(env_, {fn1}));
    OnStateChange(env_, NapiMock::MakeCbInfo(env_, {fn2}));
    ASSERT_EQ(g_stateChangeCbRefs.size(), 2u);

    // 移除 fn1：fn1 同一对象匹配成功
    OffStateChange(env_, NapiMock::MakeCbInfo(env_, {fn1}));
    EXPECT_FALSE(NapiMock::HasPendingException(env_));
    EXPECT_EQ(g_stateChangeCbRefs.size(), 1u);

    // 未注册的函数：不匹配、不影响列表
    napi_value fnUnknown = NapiMock::MakeFunction(env_);
    OffStateChange(env_, NapiMock::MakeCbInfo(env_, {fnUnknown}));
    EXPECT_EQ(g_stateChangeCbRefs.size(), 1u);
    // 剩余为 fn2
    napi_value remaining = nullptr;
    napi_get_reference_value(env_, g_stateChangeCbRefs[0], &remaining);
    EXPECT_EQ(remaining, fn2);
}

/**
 * @tc.name  : OffStateChangeUnregisterAllTest018
 * @tc.number: EthEapNapiConfigTest_018
 * @tc.desc  : offStateChange 不带参数：整体注销（服务端注销成功时清空全部引用与回调对象）
 */
HWTEST_F(EthEapNapiConfigTest, OffStateChangeUnregisterAllTest018, TestSize.Level1)
{
    ASSERT_TRUE(CreateStateChangeTsf(env_));
    g_stateCallback = sptr<EthEapStateNapiCallback>::MakeSptr();
    OnStateChange(env_, NapiMock::MakeCbInfo(env_, {NapiMock::MakeFunction(env_)}));
    ASSERT_EQ(g_stateChangeCbRefs.size(), 1u);

    OffStateChange(env_, NapiMock::MakeCbInfo(env_, {}));
    // 真机：服务端因无权限返回 201 → 抛出业务码；无服务环境：33200009。
    // 服务端注销成功的环境：清空引用与全局回调对象
    if (!NapiMock::HasPendingException(env_)) {
        EXPECT_TRUE(g_stateChangeCbRefs.empty());
        EXPECT_EQ(g_stateCallback, nullptr);
    } else {
        EXPECT_TRUE(IsEnvDependentClientCode(NapiMock::PendingExceptionCode(env_)));
    }
}

/**
 * @tc.name  : ConnectParamValidationTest019
 * @tc.number: EthEapNapiConfigTest_019
 * @tc.desc  : connect action 参数校验：缺失/非数字/非法枚举值均拒绝为 401
 */
HWTEST_F(EthEapNapiConfigTest, ConnectParamValidationTest019, TestSize.Level1)
{
    // 无参数
    napi_value promise = Connect(env_, NapiMock::MakeCbInfo(env_, {}));
    ASSERT_NE(promise, nullptr);
    EXPECT_EQ(NapiMock::RejectedCount(env_), 1u);
    EXPECT_EQ(NapiMock::GetProp(env_, NapiMock::LastRejectedValue(env_), "code")->intVal, EAP_ERRCODE_PARAMETER_ERROR);

    // 非数字
    NapiMock::ResetEnvRecords(env_);
    promise = Connect(env_, NapiMock::MakeCbInfo(env_, {NapiMock::MakeString(env_, "one")}));
    EXPECT_EQ(NapiMock::RejectedCount(env_), 1u);
    EXPECT_EQ(NapiMock::GetProp(env_, NapiMock::LastRejectedValue(env_), "code")->intVal, EAP_ERRCODE_PARAMETER_ERROR);

    // 非法枚举值（5）
    NapiMock::ResetEnvRecords(env_);
    promise = Connect(env_, NapiMock::MakeCbInfo(env_, {NapiMock::MakeNumber(env_, 5)}));
    EXPECT_EQ(NapiMock::RejectedCount(env_), 1u);
    EXPECT_EQ(NapiMock::GetProp(env_, NapiMock::LastRejectedValue(env_), "code")->intVal, EAP_ERRCODE_PARAMETER_ERROR);
}

/**
 * @tc.name  : ConnectValidActionTest020
 * @tc.number: EthEapNapiConfigTest_020
 * @tc.desc  : connect 合法 action（0/1）透传后 Promise 恰好兑现一次（环境相关业务码拒绝或成功）
 */
HWTEST_F(EthEapNapiConfigTest, ConnectValidActionTest020, TestSize.Level1)
{
    napi_value promise = Connect(env_, NapiMock::MakeCbInfo(env_, {NapiMock::MakeNumber(env_, 0)}));
    ASSERT_NE(promise, nullptr);
    EXPECT_EQ(NapiMock::ResolvedCount(env_) + NapiMock::RejectedCount(env_), 1u);
    if (NapiMock::RejectedCount(env_) == 1u) {
        EXPECT_TRUE(IsEnvDependentClientCode(NapiMock::GetProp(env_, NapiMock::LastRejectedValue(env_), "code")->intVal));
    }

    NapiMock::ResetEnvRecords(env_);
    promise = Connect(env_, NapiMock::MakeCbInfo(env_, {NapiMock::MakeNumber(env_, 1)}));
    ASSERT_NE(promise, nullptr);
    EXPECT_EQ(NapiMock::ResolvedCount(env_) + NapiMock::RejectedCount(env_), 1u);
}

/**
 * @tc.name  : OnEthEapStateChangedNoTsfTest021
 * @tc.number: EthEapNapiConfigTest_021
 * @tc.desc  : 投递通道未初始化时状态事件直接丢弃并返回内部错误（不触碰 napi 对象）
 */
HWTEST_F(EthEapNapiConfigTest, OnEthEapStateChangedNoTsfTest021, TestSize.Level1)
{
    napi_threadsafe_function savedTsf = g_stateChangeTsf;
    g_stateChangeTsf = nullptr;
    auto callback = sptr<EthEapStateNapiCallback>::MakeSptr();
    ASSERT_NE(callback, nullptr);
    EthEapStateInfo stateInfo;
    stateInfo.state = EthEapState::STATE_AUTHENTICATING;
    EXPECT_EQ(callback->OnEthEapStateChanged(stateInfo), NETMANAGER_EXT_ERR_INTERNAL);
    g_stateChangeTsf = savedTsf;
}

/**
 * @tc.name  : StateChangePipelineTest022
 * @tc.number: EthEapNapiConfigTest_022
 * @tc.desc  : 状态事件经线程安全投递通道完整回放：JS 回调收到 {state,retryCount,message} 参数
 */
HWTEST_F(EthEapNapiConfigTest, StateChangePipelineTest022, TestSize.Level1)
{
    // 建通道 + 预置回调对象 + 注册 2 个 JS 回调（全部确定性路径）
    ASSERT_TRUE(CreateStateChangeTsf(env_));
    g_stateCallback = sptr<EthEapStateNapiCallback>::MakeSptr();
    ASSERT_NE(g_stateCallback, nullptr);
    napi_value fn1 = NapiMock::MakeFunction(env_);
    napi_value fn2 = NapiMock::MakeFunction(env_);
    OnStateChange(env_, NapiMock::MakeCbInfo(env_, {fn1}));
    OnStateChange(env_, NapiMock::MakeCbInfo(env_, {fn2}));
    ASSERT_EQ(g_stateChangeCbRefs.size(), 2u);
    size_t callsBefore = NapiMock::JsCallCount(env_);

    EthEapStateInfo stateInfo;
    stateInfo.state = EthEapState::STATE_AUTHENTICATED;
    stateInfo.retryCount = 0;
    stateInfo.message = "pipeline-dt";
    // 模拟同步投递到 JS 线程：两个 JS 回调各被调用一次
    ASSERT_EQ(g_stateCallback->OnEthEapStateChanged(stateInfo), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(NapiMock::JsCallCount(env_), callsBefore + 2);

    auto *lastCall = NapiMock::LastJsCall(env_);
    ASSERT_NE(lastCall, nullptr);
    ASSERT_EQ(lastCall->argv.size(), 1u);
    napi_value__ *infoObj = lastCall->argv[0];
    ASSERT_NE(infoObj, nullptr);
    EXPECT_EQ(NapiMock::GetProp(env_, infoObj, "state")->intVal, static_cast<int32_t>(EthEapState::STATE_AUTHENTICATED));
    EXPECT_EQ(NapiMock::GetProp(env_, infoObj, "retryCount")->intVal, 0);
    EXPECT_EQ(NapiMock::GetProp(env_, infoObj, "message")->strVal, "pipeline-dt");
}

/**
 * @tc.name  : StateChangeCallJsNoRefsTest023
 * @tc.number: EthEapNapiConfigTest_023
 * @tc.desc  : JS 回调列表为空或事件数据为空时投递体安全丢弃（不崩溃、不调用 JS）
 */
HWTEST_F(EthEapNapiConfigTest, StateChangeCallJsNoRefsTest023, TestSize.Level1)
{
    size_t callsBefore = NapiMock::JsCallCount(env_);
    // 数据为空：直接丢弃
    StateChangeCallJs(env_, nullptr, nullptr, nullptr);
    EXPECT_EQ(NapiMock::JsCallCount(env_), callsBefore);

    // 无已注册回调：仅丢弃数据
    EthEapStateInfo stateInfo;
    stateInfo.state = EthEapState::STATE_IDLE;
    StateChangeCallJs(env_, nullptr, nullptr, &stateInfo);
    EXPECT_EQ(NapiMock::JsCallCount(env_), callsBefore);
}

#endif // NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

} // namespace NetManagerStandard
} // namespace OHOS
