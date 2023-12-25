/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_SERVICE_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_SERVICE_EXTENSION_CONTEXT_H

#include "extension_context.h"

#include "ability_connect_callback.h"
#include "connection_manager.h"
#include "local_call_container.h"
#include "start_options.h"
#include "want.h"
#include "runtime.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace AbilityRuntime;
/**
 * @brief context supply for vpn
 *
 */
class VpnExtensionContext : public ExtensionContext {
public:
    VpnExtensionContext() = default;
    virtual ~VpnExtensionContext() = default;

    /**
     * clear failed call connection by callback object
     *
     * @param callback Indicates the callback object.
     *
     * @return void.
     */
    void ClearFailedCallConnection(const std::shared_ptr<CallerCallBack> &callback) const;

    /**
     * @brief Connects the current ability to an ability using the AbilityInfo.AbilityType.SERVICE template.
     *
     * @param want Indicates the want containing information about the ability to connect
     *
     * @param conn Indicates the callback object when the target ability is connected.
     *
     * @return Returns zero on success, others on failure.
     */
    ErrCode ConnectAbility(
        const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const;

    ErrCode StartVpnExtensionAbility(const AAFwk::Want &want, int32_t accountId = -1) const;

    ErrCode StopVpnExtensionAbility(const AAFwk::Want& want, int32_t accountId = -1) const;

    using SelfType = VpnExtensionContext;
    static const size_t CONTEXT_TYPE_ID;

protected:
    bool IsContext(size_t contextTypeId) override
    {
        return contextTypeId == CONTEXT_TYPE_ID || ExtensionContext::IsContext(contextTypeId);
    }

private:
    static int ILLEGAL_REQUEST_CODE;
    std::shared_ptr<LocalCallContainer> localCallContainer_ = nullptr;
};
}  // namespace NetManagerStandard
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_SERVICE_EXTENSION_CONTEXT_H
