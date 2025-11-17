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
 
#include <algorithm>
#include "napi/native_node_api.h"
#include "eap_event_mgr.h"
namespace OHOS {
namespace NetManagerStandard {
 
static constexpr int32_t REGISTERINFO_MAX_NUM = 16;
static constexpr int32_t WIFI_DEVICE_SA_ID = 1120;
static constexpr int32_t COMM_ETHERNET_MANAGER_SYS_ABILITY_ID = 1157;
static constexpr uint32_t INVALID_REF_COUNT = 0xff;
static constexpr const char *EAP_BUFFER = "eapBuffer";
static constexpr const char *EAP_BUFFERLEN = "bufferLen";
static constexpr const char *EAP_MSGID = "msgId";
static constexpr const int32_t EAP_CODE_SUCCESS = 3;
static constexpr const int32_t EAP_CODE_FAILURE = 4;
static constexpr const int32_t OFFSET_EAPCODE = 8;
static std::shared_mutex g_regInfoMutex;
 
int32_t NetEapPostBackCallback::OnEapSupplicantPostback(NetType netType, const sptr<EapData> &eapData)
{
    if (eapData == nullptr) {
        NETMANAGER_EXT_LOGE("%{public}s eapData is nullptr.", __func__);
        return NETMANAGER_ERR_LOCAL_PTR_NULL;
    }
    if (eapData->eapCode < EAP_CODE_MIN || eapData->eapCode > EAP_CODE_MAX) {
        NETMANAGER_EXT_LOGE("eapCode %{public}d invalid.", eapData->eapCode);
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    if (eapData->eapBuffer.size() == 0) {
        NETMANAGER_EXT_LOGE("%{public}s eapBuffer size is 0. %{public}s", __func__, eapData->PrintLogInfo().c_str());
        return NETMANAGER_ERR_PARAMETER_ERROR;
    }
    NETMANAGER_EXT_LOGI("%{public}s: eapCode:%{public}d, eapType:%{public}d, buffsize:%{public}zu", __func__,
        eapData->eapCode, eapData->eapType, eapData->eapBuffer.size());
    uint32_t composeParam =  (eapData->eapCode << OFFSET_EAPCODE) | eapData->eapType;
    if (eapData->eapCode == EAP_CODE_SUCCESS || eapData->eapCode == EAP_CODE_FAILURE) {
        composeParam = (eapData->eapCode << OFFSET_EAPCODE);
    }
    return CheckAndNotifyApp(netType, composeParam, eapData);
}
 
napi_value NetEapPostBackCallback::CreateResult(const napi_env& env, const sptr<EapData> &eapData)
{
    napi_value obj = NapiUtils::CreateObject(env);
    if (NapiUtils::GetValueType(env, obj) != napi_object) {
        return NapiUtils::GetUndefined(env);
    }
 
    NapiUtils::SetVectorUint8Property(env, obj, EAP_BUFFER, eapData->eapBuffer);
    NapiUtils::SetUint32Property(env, obj, EAP_BUFFERLEN, eapData->bufferLen);
    NapiUtils::SetUint32Property(env, obj, EAP_MSGID, eapData->msgId);
    return obj;
}
 
bool NetEapPostBackCallback::CheckAndNotifyApp(NetType netType, const int32_t key, const sptr<EapData> &eapData)
{
    std::shared_lock<std::shared_mutex> lock(g_regInfoMutex);
    auto regInfo = EapEventMgr::GetInstance().GetRegisterInfoMap();
    auto netTypeIter = regInfo.find(netType);
    if (netTypeIter == regInfo.end()) {
        NETMANAGER_EXT_LOGE("%{public}s, netType %{public}d not find register info.", __func__, netType);
        return false;
    }
    auto it = netTypeIter->second.find(key);
    if (it == netTypeIter->second.end()) {
        NETMANAGER_EXT_LOGE("%{public}s, not find register info.", __func__);
        return false;
    }
    for (auto& each : it->second) {
        auto func = [this, env = each.m_regEnv, eapData] () -> napi_value { return this->CreateResult(env, eapData); };
        std::shared_ptr<AsyncEventData> asyncEvent =
            std::make_shared<AsyncEventData>(each.m_regEnv, each.m_regHandlerRef, func);
        asyncEvent->Init(key, netType, eapData->msgId);
        EventNotify(asyncEvent);
    }
    return true;
}
 
void NetEapPostBackCallback::SendTask(const std::shared_ptr<AsyncEventData> &asyncEvent)
{
    napi_value handler = nullptr;
    napi_value jsEvent = nullptr;
    napi_value undefine;
    uint32_t refCount = INVALID_REF_COUNT;
    napi_status res;
    InitScope(asyncEvent);
    auto regInfo = EapEventMgr::GetInstance().GetRegisterInfoMap();
    auto it = regInfo[asyncEvent->netType_].find(asyncEvent->key_);
    if (it == regInfo[asyncEvent->netType_].end()) {
        NETMANAGER_EXT_LOGE("%{public}s, event has been unregistered.", __func__);
        EndSendTask(asyncEvent, false, refCount);
        return;
    }
    for (auto& each : it->second) {
        if (each.m_regEnv == asyncEvent->env_ &&
            each.m_regHandlerRef == asyncEvent->callbackRef_ && each.refCount_ > 0) {
            res = napi_reference_ref(asyncEvent->env_, asyncEvent->callbackRef_, &each.refCount_);
            NETMANAGER_EXT_LOGI("%{public}s, res: %{public}d, callbackRef: %{private}p, refCount: %{public}d",
                __func__, res, asyncEvent->callbackRef_, each.refCount_);
            if (res != napi_ok || each.refCount_ <= 1) {
                EndSendTask(asyncEvent, true, each.refCount_);
                return;
            }
            res = napi_get_reference_value(asyncEvent->env_, asyncEvent->callbackRef_, &handler);
            if (res != napi_ok || handler == nullptr) {
                NETMANAGER_EXT_LOGE("%{public}s, handler is nullptr or res: %{public}d!", __func__, res);
                EndSendTask(asyncEvent, true, each.refCount_);
                return;
            }
            napi_get_undefined(asyncEvent->env_, &undefine);
            jsEvent = asyncEvent->packResult_();
            if (napi_call_function(asyncEvent->env_, nullptr, handler, 1, &jsEvent, &undefine) != napi_ok) {
                NETMANAGER_EXT_LOGE("%{public}s, Report event to Js failed", __func__);
            }
            EndSendTask(asyncEvent, true, each.refCount_);
            return;
        }
    }
    NETMANAGER_EXT_LOGE("%{public}s, NOT find the event.", __func__);
    EndSendTask(asyncEvent, false, refCount);
    return;
}
 
void NetEapPostBackCallback::InitScope(const std::shared_ptr<AsyncEventData> &asyncEvent)
{
    napi_open_handle_scope(asyncEvent->env_, &scope_);
    uint32_t refCount = 0;
    if (scope_ == nullptr) {
        NETMANAGER_EXT_LOGE("napi_send_event, scope is nullptr");
        EndSendTask(asyncEvent, false, refCount);
    }
}
 
void NetEapPostBackCallback::EndSendTask(const std::shared_ptr<AsyncEventData> &asyncEvent,
    bool unrefRef, uint32_t &refCount)
{
    napi_close_handle_scope(asyncEvent->env_, scope_);
    if (unrefRef && refCount > 0) {
        napi_reference_unref(asyncEvent->env_, asyncEvent->callbackRef_, &refCount);
    }
}
 
void NetEapPostBackCallback::EventNotify(const std::shared_ptr<AsyncEventData> &asyncEvent)
{
    if (asyncEvent == nullptr) {
        NETMANAGER_EXT_LOGE("asyncEvent is null!");
        return;
    }
    auto sendTask = std::bind(&NetEapPostBackCallback::SendTask, this, asyncEvent);
    if (napi_status::napi_ok != napi_send_event(asyncEvent->env_, sendTask, napi_eprio_immediate)) {
        NETMANAGER_EXT_LOGE("%{public}s, Failed to SendEvent", __func__);
    }
}
 
void NetManagerNapiAbilityStatusChange::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    NETMANAGER_EXT_LOGI("NetManagerNapiAbilityStatusChange OnAddSystemAbility systemAbilityId:%{public}d",
        systemAbilityId);
    switch (systemAbilityId) {
        case WIFI_DEVICE_SA_ID:
        case COMM_ETHERNET_MANAGER_SYS_ABILITY_ID:
            EapEventMgr::GetInstance().RegCustomEapHandler(NetType::WLAN0, RegTriggerMode::SA_LAUNCH);
            break;
        default:
            return;
    }
}
 
void NetManagerNapiAbilityStatusChange::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
}
 
EapEventMgr &EapEventMgr::GetInstance()
{
    static EapEventMgr instance;
    return instance;
}
 
EapEventMgr::EapEventMgr():eapPostBackCallback_(sptr<NetEapPostBackCallback>::MakeSptr())
{
        auto samgrProxy = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy == nullptr) {
            NETMANAGER_EXT_LOGE("samgrProxy is nullptr!");
            return;
        }
        sptr<NetManagerNapiAbilityStatusChange> mSaStatusListener = sptr<NetManagerNapiAbilityStatusChange>::MakeSptr();

        int32_t retWifiSa = samgrProxy->SubscribeSystemAbility((int32_t)WIFI_DEVICE_SA_ID, mSaStatusListener);
        int32_t retNetManagerSa = samgrProxy->SubscribeSystemAbility(
            (int32_t)COMM_ETHERNET_MANAGER_SYS_ABILITY_ID, mSaStatusListener);
        NETMANAGER_EXT_LOGI("EventRegister, SubscribeSystemAbility return retWifiSa:%{public}d, \
            retNetManagerSa:%{public}d!", retWifiSa, retNetManagerSa);
}
 
int32_t EapEventMgr::RegCustomEapHandler(napi_env env, NetType netType, uint32_t eapCode, uint32_t eapType,
    napi_value handler)
{
    NETMANAGER_EXT_LOGI("%{public}s enter, netType:%{public}d, eapCode:%{public}d, eapType:%{public}d", __func__,
        static_cast<int>(netType), eapCode, eapType);
    uint32_t composeParam =  (eapCode << OFFSET_EAPCODE) | eapType;
    if (eapCode == EAP_CODE_SUCCESS || eapCode == EAP_CODE_FAILURE) {
        composeParam = (eapCode << OFFSET_EAPCODE);
    }
    napi_ref handlerRef = nullptr;
    napi_create_reference(env, handler, 1, &handlerRef);
    RegObj regObj(env, handlerRef);
 
    std::unique_lock<std::shared_mutex> guard(g_regInfoMutex);
    auto netTypeMapIter = eventRegisterInfo_.find(netType);
    if (netTypeMapIter == eventRegisterInfo_.end()) {
        NETMANAGER_EXT_LOGI("%{public}s, new netType!", __func__);
        TypeMapRegObj mapObj;
        mapObj.emplace(composeParam, std::vector<RegObj>{regObj});
        eventRegisterInfo_.emplace(netType, mapObj);
    } else {
        NETMANAGER_EXT_LOGI("%{public}s, exist netType!", __func__);
        TypeMapRegObj mapObj = netTypeMapIter->second;
        auto iter = mapObj.find(composeParam);
        if (iter == mapObj.end()) {
            NETMANAGER_EXT_LOGI("%{public}s, new eapCode:%{public}d, eapType:%{public}d!", __func__,
                eapCode, eapType);
            if (mapObj.size() > REGISTERINFO_MAX_NUM) {
                NETMANAGER_EXT_LOGE("%{public}s, RegisterInfo Exceeding the maximum value!", __func__);
                return EAP_ERRCODE_INTERNAL_ERROR;
            }
            mapObj.emplace(composeParam, std::vector<RegObj>{regObj});
            eventRegisterInfo_[netType] = mapObj;
        } else {
            auto vecIter = std::find_if(iter->second.begin(), iter->second.end(),
                [&regObj] (const RegObj &obj) { return regObj.m_regEnv == obj.m_regEnv;});
            if (vecIter != iter->second.end()) {
                NETMANAGER_EXT_LOGE("%{public}s, eapCode:%{public}d, eapType:%{public}d callback is registered!",
                    __func__, eapCode, eapType);
                return EAP_ERRCODE_SUCCESS;
            }
            iter->second.emplace_back(regObj);
            NETMANAGER_EXT_LOGI("%{public}s, eapCode:%{public}d, eapType:%{public}d callback size:%{public}zu!",
                __func__, eapCode, eapType, iter->second.size());
        }
    }
    guard.unlock();
    return RegCustomEapHandler(netType, RegTriggerMode::USER_REGISTER);
}
 
int32_t EapEventMgr::RegCustomEapHandler(NetType netType, RegTriggerMode triggerMode)
{
    std::string regCmd;
    {
        std::shared_lock<std::shared_mutex> guard(g_regInfoMutex);
        auto mNetTypeValueIter = eventRegisterInfo_.find(netType);
        if (mNetTypeValueIter == eventRegisterInfo_.end()) {
            NETMANAGER_EXT_LOGE("%{public}s eventRegisterInfo_ not have eapType:%{public}d", __func__, netType);
            return EAP_ERRCODE_INTERNAL_ERROR;
        }
        regCmd += std::to_string(static_cast<int>(netType));
        regCmd += ":";
        regCmd += std::to_string(mNetTypeValueIter->second.size());
        for (auto &iter : mNetTypeValueIter->second) {
            regCmd += ":";
            regCmd += std::to_string(iter.first);
        }
    }
    NETMANAGER_EXT_LOGI("%{public}s enter, triggreMode:%{public}d, netType:%{public}d, regCmd:%{public}s", __func__,
        static_cast<int>(triggerMode), static_cast<int>(netType), regCmd.c_str());
    return DelayedSingleton<EthernetClient>::GetInstance()->RegCustomEapHandler(netType, regCmd,
        eapPostBackCallback_);
}
 
int32_t EapEventMgr::UnRegCustomEapHandler(napi_env env, NetType netType, uint32_t eapCode, uint32_t eapType,
    napi_value handler)
{
    NETMANAGER_EXT_LOGI("%{public}s enter, netType:%{public}d, eapCode:%{public}d, eapType:%{public}d", __func__,
        static_cast<int>(netType), eapCode, eapType);
    uint32_t composeParam =  (eapCode << OFFSET_EAPCODE) | eapType;
    if (eapCode == EAP_CODE_SUCCESS || eapCode == EAP_CODE_FAILURE) {
        composeParam = (eapCode << OFFSET_EAPCODE);
    }
    bool needUnregister = false;
    {
        std::unique_lock<std::shared_mutex> guard(g_regInfoMutex);
        auto netTypeMapIter = eventRegisterInfo_.find(netType);
        if (netTypeMapIter == eventRegisterInfo_.end()) {
            NETMANAGER_EXT_LOGE("%{public}s, not netType %{public}d handler", __func__, netType);
            return EAP_ERRCODE_INTERNAL_ERROR;
        }
        TypeMapRegObj& mapObj = netTypeMapIter->second;
        auto mapObjIter = mapObj.find(composeParam);
        if (mapObjIter == mapObj.end()) {
            NETMANAGER_EXT_LOGE("%{public}s, not composeParam %{public}d handler", __func__, composeParam);
            return EAP_ERRCODE_INTERNAL_ERROR;
        }
        auto new_end = std::remove_if(mapObjIter->second.begin(), mapObjIter->second.end(),
            [env](RegObj& obj) {
                if (obj.m_regEnv == env && obj.refCount_ > 0) {
                    napi_status status = napi_reference_unref(obj.m_regEnv, obj.m_regHandlerRef, &obj.refCount_);
                    if (status == napi_ok && obj.refCount_ == 0) {
                        obj.m_regHandlerRef = NULL;
                    }
                    return true;
                }
                return false;
            });
        mapObjIter->second.erase(new_end, mapObjIter->second.end());
    // if have no callbacks, supplicant unregister this eap code and type
        if (mapObjIter->second.size() == 0) {
            mapObj.erase(mapObjIter);
            needUnregister = true;
        }
    }
    if (needUnregister) {
        NETMANAGER_EXT_LOGI("%{public}s, eapCode:%{public}d, eapType:%{public}d", __func__, eapCode, eapType);
        return UnRegCustomEapHandler(netType);
    }
    return true;
}
 
int32_t EapEventMgr::UnRegCustomEapHandler(NetType netType)
{
    return RegCustomEapHandler(netType, RegTriggerMode::UNREGISTER);
}
 
int32_t EapEventMgr::ReplyCustomEapData(CustomResult result, const sptr<EapData> &eapData)
{
    return DelayedSingleton<EthernetClient>::GetInstance()->ReplyCustomEapData(static_cast<int>(result),
        eapData);
}

std::map<NetType, TypeMapRegObj>& EapEventMgr::GetRegisterInfoMap()
{
    return eventRegisterInfo_;
}

EapEventMgr::~EapEventMgr()
{
    auto deleteReference = [](RegObj &obj) {
        if (obj.refCount_ > 0) {
            napi_status status = napi_delete_reference(obj.m_regEnv, obj.m_regHandlerRef);
            if (status == napi_ok) {
                obj.refCount_ = 0;
                obj.m_regHandlerRef = NULL;
            }
        }
    };
    for (const auto iterNetType : eventRegisterInfo_) {
        for (const auto iterRegObj : iterNetType.second) {
            for (auto obj : iterRegObj.second) {
                    deleteReference(obj);
            }
        }
    }
}

} // namespace NetManagerStandard
} // namespace OHOS