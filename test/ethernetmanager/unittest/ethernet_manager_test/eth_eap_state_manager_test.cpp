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

#include <atomic>

#include "eth_eap_state_manager.h"
#include "eth_eap_state_callback_proxy.h"
#include "eth_eap_state_callback_stub.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

using namespace testing::ext;

namespace {
/** 测试用状态回调：记录最近一次收到的状态信息与回调次数 */
class EthEapStateCallbackTestImpl : public EthEapStateCallbackStub {
public:
    int32_t OnEthEapStateChanged(const EthEapStateInfo &stateInfo) override
    {
        lastInfo = stateInfo;
        ++callCount;
        return NETMANAGER_EXT_SUCCESS;
    }

    EthEapStateInfo lastInfo;
    std::atomic<int32_t> callCount{0};
};
} // namespace

class EthEapStateManagerTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        auto &mgr = EthEapStateManager::GetInstance();
        mgr.SetRetryCount(0);
        mgr.SetState(EthEapState::STATE_IDLE, "");
    }
    void TearDown() override
    {
        auto &mgr = EthEapStateManager::GetInstance();
        mgr.SetRetryCount(0);
        mgr.SetState(EthEapState::STATE_IDLE, "");
    }
};

/**
 * @tc.name  : SetStateTransitionTest001
 * @tc.number: EthEapStateManagerTest_001
 * @tc.desc  : SetState 状态迁移对所有状态枚举生效，GetState 返回最新状态
 */
HWTEST_F(EthEapStateManagerTest, SetStateTransitionTest001, TestSize.Level1)
{
    auto &mgr = EthEapStateManager::GetInstance();
    mgr.SetState(EthEapState::STATE_AUTHENTICATING);
    EXPECT_EQ(mgr.GetState(), EthEapState::STATE_AUTHENTICATING);
    mgr.SetState(EthEapState::STATE_AUTHENTICATED);
    EXPECT_EQ(mgr.GetState(), EthEapState::STATE_AUTHENTICATED);
    mgr.SetState(EthEapState::STATE_RETRYING);
    EXPECT_EQ(mgr.GetState(), EthEapState::STATE_RETRYING);
    mgr.SetState(EthEapState::STATE_FAILED);
    EXPECT_EQ(mgr.GetState(), EthEapState::STATE_FAILED);
    mgr.SetState(EthEapState::STATE_IDLE);
    EXPECT_EQ(mgr.GetState(), EthEapState::STATE_IDLE);
}

/**
 * @tc.name  : GetStateInfoSnapshotTest002
 * @tc.number: EthEapStateManagerTest_002
 * @tc.desc  : GetStateInfo 原子读取状态/重试计数/消息的一致性快照
 */
HWTEST_F(EthEapStateManagerTest, GetStateInfoSnapshotTest002, TestSize.Level1)
{
    auto &mgr = EthEapStateManager::GetInstance();
    mgr.SetRetryCount(2);
    mgr.SetState(EthEapState::STATE_FAILED, "AUTH_TIMEOUT");
    EthEapStateInfo info;
    mgr.GetStateInfo(info);
    EXPECT_EQ(info.state, EthEapState::STATE_FAILED);
    EXPECT_EQ(info.retryCount, 2);
    EXPECT_EQ(info.message, "AUTH_TIMEOUT");
}

/**
 * @tc.name  : RetryCountOperationsTest003
 * @tc.number: EthEapStateManagerTest_003
 * @tc.desc  : 重试计数设置/读取/复位
 */
HWTEST_F(EthEapStateManagerTest, RetryCountOperationsTest003, TestSize.Level1)
{
    auto &mgr = EthEapStateManager::GetInstance();
    EXPECT_EQ(mgr.GetRetryCount(), 0);
    mgr.SetRetryCount(3);
    EXPECT_EQ(mgr.GetRetryCount(), 3);
    mgr.ResetRetryCount();
    EXPECT_EQ(mgr.GetRetryCount(), 0);
}

/**
 * @tc.name  : RegisterCallbackNullTest004
 * @tc.number: EthEapStateManagerTest_004
 * @tc.desc  : 注册/注销空回调返回参数错误
 */
HWTEST_F(EthEapStateManagerTest, RegisterCallbackNullTest004, TestSize.Level1)
{
    auto &mgr = EthEapStateManager::GetInstance();
    sptr<IEthEapStateCallback> nullCb = nullptr;
    EXPECT_EQ(mgr.RegisterCallback(nullCb), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
    EXPECT_EQ(mgr.UnregisterCallback(nullCb), NETMANAGER_EXT_ERR_PARAMETER_ERROR);
}

/**
 * @tc.name  : RegisterUnregisterCallbackTest005
 * @tc.number: EthEapStateManagerTest_005
 * @tc.desc  : 回调注册/注销正常流程，重复注册幂等，注销未注册回调幂等返回成功
 */
HWTEST_F(EthEapStateManagerTest, RegisterUnregisterCallbackTest005, TestSize.Level1)
{
    auto &mgr = EthEapStateManager::GetInstance();
    sptr<IEthEapStateCallback> cb = new (std::nothrow) EthEapStateCallbackTestImpl();
    ASSERT_NE(cb, nullptr);
    EXPECT_EQ(mgr.RegisterCallback(cb), NETMANAGER_EXT_SUCCESS);
    // 重复注册同一回调对象：幂等，不重复入列表
    EXPECT_EQ(mgr.RegisterCallback(cb), NETMANAGER_EXT_SUCCESS);
    // 注销未注册的其他回调：幂等成功
    sptr<IEthEapStateCallback> otherCb = new (std::nothrow) EthEapStateCallbackTestImpl();
    ASSERT_NE(otherCb, nullptr);
    EXPECT_EQ(mgr.UnregisterCallback(otherCb), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(mgr.UnregisterCallback(cb), NETMANAGER_EXT_SUCCESS);
    // 再次注销已移除的回调：幂等成功
    EXPECT_EQ(mgr.UnregisterCallback(cb), NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name  : StateChangeNotifyTest006
 * @tc.number: EthEapStateManagerTest_006
 * @tc.desc  : 注册回调后 SetState 触发通知，回调收到完整状态信息
 */
HWTEST_F(EthEapStateManagerTest, StateChangeNotifyTest006, TestSize.Level1)
{
    auto &mgr = EthEapStateManager::GetInstance();
    auto *cbImpl = new (std::nothrow) EthEapStateCallbackTestImpl();
    ASSERT_NE(cbImpl, nullptr);
    sptr<IEthEapStateCallback> cb = cbImpl;
    ASSERT_EQ(mgr.RegisterCallback(cb), NETMANAGER_EXT_SUCCESS);
    int32_t beforeCount = cbImpl->callCount.load();

    mgr.SetRetryCount(1);
    mgr.SetState(EthEapState::STATE_RETRYING, "RETRY_REASON");
    EXPECT_EQ(cbImpl->callCount.load(), beforeCount + 1);
    EXPECT_EQ(cbImpl->lastInfo.state, EthEapState::STATE_RETRYING);
    EXPECT_EQ(cbImpl->lastInfo.retryCount, 1);
    EXPECT_EQ(cbImpl->lastInfo.message, "RETRY_REASON");

    // 注销后不再通知
    ASSERT_EQ(mgr.UnregisterCallback(cb), NETMANAGER_EXT_SUCCESS);
    mgr.SetState(EthEapState::STATE_IDLE, "");
    EXPECT_EQ(cbImpl->callCount.load(), beforeCount + 1);
}

/**
 * @tc.name  : EthEapStateInfoParcelRoundtripTest007
 * @tc.number: EthEapStateManagerTest_007
 * @tc.desc  : EthEapStateInfo Parcel 序列化/反序列化往返一致
 */
HWTEST_F(EthEapStateManagerTest, EthEapStateInfoParcelRoundtripTest007, TestSize.Level1)
{
    EthEapStateInfo in;
    in.state = EthEapState::STATE_RETRYING;
    in.retryCount = 2;
    in.message = "parcel-roundtrip-message";
    Parcel parcel;
    ASSERT_TRUE(in.Marshalling(parcel));

    EthEapStateInfo *out = EthEapStateInfo::Unmarshalling(parcel);
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(out->state, EthEapState::STATE_RETRYING);
    EXPECT_EQ(out->retryCount, 2);
    EXPECT_EQ(out->message, "parcel-roundtrip-message");
    delete out;
}

/**
 * @tc.name  : EthEapStateCallbackStubDispatchTest008
 * @tc.number: EthEapStateManagerTest_008
 * @tc.desc  : 状态回调 Stub 消息分发：接口描述符不符返回失败，合法请求回调收到状态信息
 */
HWTEST_F(EthEapStateManagerTest, EthEapStateCallbackStubDispatchTest008, TestSize.Level1)
{
    auto *cbImpl = new (std::nothrow) EthEapStateCallbackTestImpl();
    ASSERT_NE(cbImpl, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    constexpr uint32_t CODE_ON_STATE_CHANGED = 0;

    // 未写接口描述符：校验失败
    EthEapStateInfo info;
    info.state = EthEapState::STATE_AUTHENTICATING;
    info.retryCount = 0;
    info.message = "stub-dispatch";
    ASSERT_TRUE(data.WriteInterfaceToken(u"OHOS.NetManagerStandard.Not.This.Interface"));
    ASSERT_TRUE(info.Marshalling(data));
    EXPECT_EQ(cbImpl->OnRemoteRequest(CODE_ON_STATE_CHANGED, data, reply, option),
              NETMANAGER_EXT_ERR_OPERATION_FAILED);
    EXPECT_EQ(cbImpl->callCount.load(), 0);

    // 合法请求：描述符正确，回调收到完整状态信息
    MessageParcel data2;
    ASSERT_TRUE(data2.WriteInterfaceToken(IEthEapStateCallback::GetDescriptor()));
    ASSERT_TRUE(info.Marshalling(data2));
    EXPECT_EQ(cbImpl->OnRemoteRequest(CODE_ON_STATE_CHANGED, data2, reply, option), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(cbImpl->callCount.load(), 1);
    EXPECT_EQ(cbImpl->lastInfo.state, EthEapState::STATE_AUTHENTICATING);
    EXPECT_EQ(cbImpl->lastInfo.message, "stub-dispatch");

    // 未知消息码：不崩溃（交由基类处理）
    MessageParcel data3;
    ASSERT_TRUE(data3.WriteInterfaceToken(IEthEapStateCallback::GetDescriptor()));
    EXPECT_NO_FATAL_FAILURE(cbImpl->OnRemoteRequest(9999, data3, reply, option));
}

/**
 * @tc.name  : EthEapStateCallbackStubEmptyParcelTest009
 * @tc.number: EthEapStateManagerTest_009
 * @tc.desc  : 状态回调 Stub 收到无状态数据的请求时返回空指针错误，不崩溃
 */
HWTEST_F(EthEapStateManagerTest, EthEapStateCallbackStubEmptyParcelTest009, TestSize.Level1)
{
    auto *cbImpl = new (std::nothrow) EthEapStateCallbackTestImpl();
    ASSERT_NE(cbImpl, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    constexpr uint32_t CODE_ON_STATE_CHANGED = 0;
    // 仅写描述符，不写状态信息：Unmarshalling 失败
    ASSERT_TRUE(data.WriteInterfaceToken(IEthEapStateCallback::GetDescriptor()));
    EXPECT_EQ(cbImpl->OnRemoteRequest(CODE_ON_STATE_CHANGED, data, reply, option),
              NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);
    EXPECT_EQ(cbImpl->callCount.load(), 0);
}

/**
 * @tc.name  : EthEapStateCallbackProxyNullRemoteTest010
 * @tc.number: EthEapStateManagerTest_010
 * @tc.desc  : 状态回调 Proxy 对端为空时序列化后返回空指针错误，不崩溃
 */
HWTEST_F(EthEapStateManagerTest, EthEapStateCallbackProxyNullRemoteTest010, TestSize.Level1)
{
    sptr<IRemoteObject> nullRemote = nullptr;
    EthEapStateCallbackProxy proxy(nullRemote);
    EthEapStateInfo info;
    info.state = EthEapState::STATE_AUTHENTICATED;
    info.retryCount = 0;
    info.message = "proxy-null-remote";
    EXPECT_EQ(proxy.OnEthEapStateChanged(info), NETMANAGER_EXT_ERR_LOCAL_PTR_NULL);
}

#endif // NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

} // namespace NetManagerStandard
} // namespace OHOS
