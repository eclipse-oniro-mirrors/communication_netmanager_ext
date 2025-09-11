/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#ifndef COMMUNICATION_NET_MANAGER_BASE_EAP_EVENT_MANAGER_H
#define COMMUNICATION_NET_MANAGER_BASE_EAP_EVENT_MANAGER_H
 
#include <napi/native_api.h>
#include <map>
#include <vector>
#include <memory>
#include "ethernet_client.h"
#include "napi_utils.h"
#include "net_eap_callback_stub.h"
#include "netmanager_ext_log.h"
#include "system_ability_status_change_stub.h"
#include "iservice_registry.h"
 
namespace OHOS {
namespace NetManagerStandard {
 
class RegObj {
public:
    RegObj() : m_regEnv(0), m_regHandlerRef(nullptr)
    {
    }
 
    explicit RegObj(const napi_env& env, const napi_ref& ref)
    {
        m_regEnv = env;
        m_regHandlerRef = ref;
    }
 
    ~RegObj()
    {
    }
 
    bool operator == (const RegObj& other) const
    {
        return m_regEnv == other.m_regEnv && m_regHandlerRef == other.m_regHandlerRef;
    }
 
    bool operator != (const RegObj& other) const
    {
        return !(*this == other);
    }
 
    bool operator < (const RegObj& other) const
    {
        return m_regEnv < other.m_regEnv || (m_regEnv == other.m_regEnv && m_regHandlerRef < other.m_regHandlerRef);
    }
 
    napi_env m_regEnv;
    napi_ref m_regHandlerRef;
};

using TypeMapRegObj = std::map<uint32_t, std::vector<RegObj>>;

class AsyncEventData {
public:
    napi_env env_;
    napi_ref callbackRef_;
    std::function<napi_value ()> packResult_;
    int32_t key_ = 0;
    NetType netType_ = NetType::INVALID;
    int32_t msgId_ = -1;
 
    AsyncEventData(napi_env e, napi_ref r, std::function<napi_value ()> p)
    {
        env_ = e;
        callbackRef_ = r;
        packResult_ = p;
    }
    virtual ~AsyncEventData() {
    }
 
    void Init(const int32_t key, NetType netType, int32_t msgId)
    {
        key_ = key;
        netType_ = netType;
        msgId_ = msgId;
    }
};
 
class NetEapPostBackCallback : public NetEapPostbackCallbackStub {
public:
    NetEapPostBackCallback() = default;
    virtual ~NetEapPostBackCallback() = default;
 
public:
    int32_t OnEapSupplicantPostback(NetType netType, const sptr<EapData> &eapData) override;
 
private:
    bool CheckAndNotifyApp(NetType netType, const int32_t key, const sptr<EapData> &eapData);
    void EventNotify(const std::shared_ptr<AsyncEventData> &asyncEvent);
    void SendTask(const std::shared_ptr<AsyncEventData> &asyncEvent);
    void InitScope(const std::shared_ptr<AsyncEventData> &asyncEvent);
    void EndSendTask(const std::shared_ptr<AsyncEventData> &asyncEvent, bool unrefRef, uint32_t refCount);
    napi_value CreateResult(const napi_env& env, const sptr<EapData> &eapData);
 
private:
    napi_handle_scope scope_ = nullptr;
};
 
class NetManagerNapiAbilityStatusChange : public SystemAbilityStatusChangeStub {
public:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
};
 
class EapEventMgr final {
public:
    EapEventMgr();
    ~EapEventMgr() = default;
 
    static EapEventMgr &GetInstance();
    int32_t RegCustomEapHandler(napi_env env, NetType netType, uint32_t eapCode, uint32_t eapType, napi_value handler);
    int32_t UnRegCustomEapHandler(napi_env env, NetType netType, uint32_t eapCode, uint32_t eapType,
        napi_value handler);
    int32_t ReplyCustomEapData(CustomResult result, const sptr<EapData> &eapData);
    int32_t RegCustomEapHandler(NetType netType, RegTriggerMode triggerMode);
    int32_t UnRegCustomEapHandler(NetType netType);
    std::map<NetType, TypeMapRegObj>& GetRegisterInfoMap();
 
private:
    sptr<NetEapPostBackCallback> eapPostBackCallback_ = nullptr;
    OHOS::sptr<OHOS::ISystemAbilityStatusChange> mSaStatusListener = nullptr;
    std::map<NetType, TypeMapRegObj> eventRegisterInfo_;
};
} // namespace NetManagerStandard
} // namespace OHOS
 
#endif /* COMMUNICATION_NET_MANAGER_BASE_EAP_EVENT_MANAGER_H */