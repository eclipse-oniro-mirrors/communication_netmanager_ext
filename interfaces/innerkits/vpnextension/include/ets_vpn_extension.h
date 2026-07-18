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

#ifndef OHOS_ABILITY_RUNTIME_ETS_VPN_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_ETS_VPN_EXTENSION_H

#include "configuration.h"
#ifdef SUPPORT_GRAPHICS
#include "system_ability_status_change_stub.h"
#endif
#include "ani.h"
#include "ani_common_configuration.h"
#include "ani_common_util.h"
#include "display_manager.h"
#include "ets_native_reference.h"
#include "ets_runtime.h"
#include "remote_object_taihe_ani.h"
#include "vpn_extension.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace AbilityRuntime;

/**
 * @brief The Vpn extension in ets.
 */
class EtsVpnExtension : public VpnExtension {
public:
    explicit EtsVpnExtension(ETSRuntime& etsRuntime);
    virtual ~EtsVpnExtension() override;

    /**
     * @brief Create EtsVpnExtension.
     *
     * @param runtime The runtime.
     * @return The EtsVpnExtension instance.
     */
    static EtsVpnExtension* Create(const std::unique_ptr<Runtime>& runtime);

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
     * @brief Called when this Vpn extension is started.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the Vpn extension.
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
     * @brief Called when all abilities connected to this Vpn extension are disconnected.
     *
     * You can override this function to implement your own processing logic.
     *
     */
    virtual void OnDisconnect(const AAFwk::Want &want) override;

    /**
     * @brief Called back when Vpn is started.
     *
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
    virtual void OnConfigurationUpdated(const AppExecFwk::Configuration &configuration) override;

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

#ifdef SUPPORT_GRAPHICS
protected:
    class ETSVpnExtensionDisplayListener : public Rosen::DisplayManager::IDisplayListener {
    public:
        explicit ETSVpnExtensionDisplayListener(const std::weak_ptr<EtsVpnExtension>& etsVpnExtension)
        {
            etsVpnExtension_ = etsVpnExtension;
        }

        void OnCreate(Rosen::DisplayId displayId) override
        {
            auto sptr = etsVpnExtension_.lock();
            if (sptr != nullptr) {
                sptr->OnCreate(displayId);
            }
        }

        void OnDestroy(Rosen::DisplayId displayId) override
        {
            auto sptr = etsVpnExtension_.lock();
            if (sptr != nullptr) {
                sptr->OnDestroy(displayId);
            }
        }

        void OnChange(Rosen::DisplayId displayId) override
        {
            auto sptr = etsVpnExtension_.lock();
            if (sptr != nullptr) {
                sptr->OnChange(displayId);
            }
        }

    private:
        std::weak_ptr<EtsVpnExtension> etsVpnExtension_;
    };

    void OnCreate(Rosen::DisplayId displayId);
    void OnDestroy(Rosen::DisplayId displayId);
    void OnChange(Rosen::DisplayId displayId);

private:
    class SystemAbilityStatusChangeListener : public OHOS::SystemAbilityStatusChangeStub {
    public:
        SystemAbilityStatusChangeListener(sptr<ETSVpnExtensionDisplayListener> displayListener)
            : tmpDisplayListener_(displayListener) {};
        virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
        virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override {}

    private:
        sptr<ETSVpnExtensionDisplayListener> tmpDisplayListener_ = nullptr;
    };

    sptr<ETSVpnExtensionDisplayListener> displayListener_ = nullptr;
#endif

private:
    ani_ref CallObjectMethod(bool withResult, const char* name, const char* signature, ...);

    void BindContext(ani_env* env, std::shared_ptr<AAFwk::Want> want);

    void GetSrcPath(std::string &srcPath);

    void ListenWMS();

    ETSRuntime& etsRuntime_;
    std::unique_ptr<AppExecFwk::ETSNativeReference> etsObj_;
    std::shared_ptr<AppExecFwk::ETSNativeReference> shellContextRef_;
    std::shared_ptr<AppExecFwk::AbilityHandler> handler_ = nullptr;
};
}  // namespace NetManagerStandard
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ETS_VPN_EXTENSION_H
