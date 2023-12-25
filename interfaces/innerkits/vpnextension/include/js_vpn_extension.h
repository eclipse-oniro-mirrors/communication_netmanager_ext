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

#ifndef OHOS_NETMANAGER_EXT_JS_VPN_EXTENSION_H
#define OHOS_NETMANAGER_EXT_JS_VPN_EXTENSION_H

#include "configuration.h"
#ifdef SUPPORT_GRAPHICS
#include "system_ability_status_change_stub.h"
#endif
#include "display_manager.h"
#include "vpn_extension.h"
#include "runtime.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

class NativeReference;

namespace OHOS {
namespace NetManagerStandard {
using namespace AbilityRuntime;
class VpnExtension;
/**
 * @brief Basic vpn components.
 */
class JsVpnExtension : public VpnExtension {
public:
    explicit JsVpnExtension(JsRuntime& jsRuntime);
    virtual ~JsVpnExtension() override;

    /**
     * @brief Create JsVpnExtension.
     *
     * @param runtime The runtime.
     * @return The JsVpnExtension instance.
     */
    static JsVpnExtension* Create(const std::unique_ptr<Runtime>& runtime);

    /**
     * @brief Init the extension.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    virtual void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Called when this extension is started. You must override this function if you want to perform some
     *        initialization operations during extension startup.
     *
     * This function can be called only once in the entire lifecycle of an extension.
     * @param Want Indicates the {@link Want} structure containing startup information about the extension.
     */
    virtual void OnStart(const AAFwk::Want &want) override;

    /**
     * @brief Called when this Vpn extension is connected for the first time.
     *
     * You can override this function to implement your own processing logic.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the Vpn extension.
     * @return Returns a pointer to the <b>sid</b> of the connected Vpn extension.
     */
    virtual sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;

    /**
     * @brief Called when this Vpn extension is connected for the first time.
     *
     * You can override this function to implement your own processing logic.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the Vpn extension.
     * @param callbackInfo Indicates the lifecycle transaction callback information
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback
     * @return Returns a pointer to the <b>sid</b> of the connected Vpn extension.
     */
    virtual sptr<IRemoteObject> OnConnect(const AAFwk::Want &want,
        AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback) override;

    /**
     * @brief Called when all abilities connected to this Vpn extension are disconnected.
     *
     * You can override this function to implement your own processing logic.
     *
     */
    virtual void OnDisconnect(const AAFwk::Want &want) override;

    /**
     * @brief Called when all abilities connected to this Vpn extension are disconnected.
     *
     * You can override this function to implement your own processing logic.
     * @param callbackInfo Indicates the lifecycle transaction callback information
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback
     */
    void OnDisconnect(const AAFwk::Want &want, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo,
        bool &isAsyncCallback) override;

    /**
     * @param want Indicates the want of Vpn to start.
     * @param restart Indicates the startup mode. The value true indicates that Vpn is restarted after being
     * destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the Vpn extension has been started. The startId is incremented
     * by 1 every time the extension is started. For example, if the extension has been started for six times, the
     * value of startId is 6.
     */
    virtual void OnCommand(const AAFwk::Want &want, bool restart, int startId) override;

    /**
     * @brief Called when this extension enters the <b>STATE_STOP</b> state.
     *
     * The extension in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnStop() override;

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdated(const AppExecFwk::Configuration& configuration) override;

    /**
     * @brief Called when configuration changed, including system configuration and window configuration.
     *
     */
    void ConfigurationUpdated();

    /**
     * @brief Called when extension need dump info.
     *
     * @param params The params from vpn.
     * @param info The dump info to show.
     */
    virtual void Dump(const std::vector<std::string> &params, std::vector<std::string> &info) override;

private:
    napi_value CallObjectMethod(const char* name, napi_value const *argv = nullptr, size_t argc = 0);

    void BindContext(napi_env env, napi_value obj);

    void GetSrcPath(std::string &srcPath);

    napi_value CallOnConnect(const AAFwk::Want &want);

    napi_value CallOnDisconnect(const AAFwk::Want &want, bool withResult = false);

    bool CheckPromise(napi_value result);

    bool CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo);

    void ListenWMS();

    JsRuntime& jsRuntime_;
    std::unique_ptr<NativeReference> jsObj_;
    std::shared_ptr<NativeReference> shellContextRef_ = nullptr;
    std::shared_ptr<AbilityHandler> handler_ = nullptr;

#ifdef SUPPORT_GRAPHICS
protected:
    class JsVpnExtensionDisplayListener : public Rosen::DisplayManager::IDisplayListener {
    public:
        explicit JsVpnExtensionDisplayListener(const std::weak_ptr<JsVpnExtension>& jsVpnExtension)
        {
            jsVpnExtension_ = jsVpnExtension;
        }

        void OnCreate(Rosen::DisplayId displayId) override
        {
            auto sptr = jsVpnExtension_.lock();
            if (sptr != nullptr) {
                sptr->OnCreate(displayId);
            }
        }

        void OnDestroy(Rosen::DisplayId displayId) override
        {
            auto sptr = jsVpnExtension_.lock();
            if (sptr != nullptr) {
                sptr->OnDestroy(displayId);
            }
        }

        void OnChange(Rosen::DisplayId displayId) override
        {
            auto sptr = jsVpnExtension_.lock();
            if (sptr != nullptr) {
                sptr->OnChange(displayId);
            }
        }

    private:
        std::weak_ptr<JsVpnExtension> jsVpnExtension_;
    };

    void OnCreate(Rosen::DisplayId displayId);
    void OnDestroy(Rosen::DisplayId displayId);
    void OnChange(Rosen::DisplayId displayId);

private:
    class SystemAbilityStatusChangeListener : public OHOS::SystemAbilityStatusChangeStub {
    public:
        SystemAbilityStatusChangeListener(sptr<JsVpnExtensionDisplayListener> displayListener)
            : tmpDisplayListener_(displayListener) {};
        virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
        virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override {}

    private:
        sptr<JsVpnExtensionDisplayListener> tmpDisplayListener_ = nullptr;
    };

    sptr<JsVpnExtensionDisplayListener> displayListener_ = nullptr;
#endif
};
}  // namespace NetManagerStandard
}  // namespace OHOS
#endif  // OHOS_NETMANAGER_EXT_JS_VPN_EXTENSION_H
