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
 
#ifndef NET_EAP_HANDLER_H
#define NET_EAP_HANDLER_H
 
#include <shared_mutex>
#include <map>
#include "eap_data.h"
#include "inet_register_eap_callback.h"
#include "inet_eap_postback_callback.h"
 
namespace OHOS {
namespace NetManagerStandard {
class NetEapHandler : public std::enable_shared_from_this<NetEapHandler> {
public:
    NetEapHandler() = default;
    ~NetEapHandler() = default;
 
    static NetEapHandler &GetInstance();
    int32_t RegisterCustomEapCallback(const NetType netType, const sptr<INetRegisterEapCallback> &callback);
    int32_t UnRegisterCustomEapCallback(const NetType netType, const sptr<INetRegisterEapCallback> &callback);
    int32_t NotifyWpaEapInterceptInfo(const NetType netType, const sptr<EapData> &eapData);
    int32_t RegCustomEapHandler(NetType netType, const std::string &regCmd,
        const sptr<INetEapPostbackCallback> &postBackCb);
    int32_t ReplyCustomEapData(int result, const sptr<EapData> &eapData);
private:
    void SetPostbackCallback(const sptr<INetEapPostbackCallback> &postbackCallback);
    sptr<INetEapPostbackCallback> GetPostbackCallback();
 
private:
    std::map<NetType, int> nTMapMsgId_;
    std::map<NetType, sptr<INetRegisterEapCallback>> regEapCallBack_;
    sptr<INetEapPostbackCallback> postbackCallback_;
    std::mutex callbackMutex_;
    std::mutex mutex_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_EAP_H